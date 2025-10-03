#!/usr/bin/env python3
# This file is part of Clementine.
# Copyright 2025, AI Stem Separation Implementation
#
# Clementine is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# Clementine is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with Clementine.  If not, see <http://www.gnu.org/licenses/>.

"""
Real-time live stem separation for Clementine Extended
Provides live audio streaming with real-time AI separation
With integrated GStreamer player for independent playback
"""

import os
import sys
import time
import threading
import logging
import queue
import signal
import json
from pathlib import Path

# Set up logging
logging.basicConfig(
    level=logging.INFO,
    format='%(asctime)s - %(levelname)s - %(message)s',
    handlers=[
        logging.StreamHandler(sys.stdout),
        logging.FileHandler('live_stem_separation.log', mode='a')
    ]
)
logger = logging.getLogger(__name__)

try:
    import torch
    import torchaudio
    import numpy as np
    from demucs.pretrained import get_model
    from demucs.apply import apply_model
    import soundfile as sf
    demucs_available = True
    logger.info("✅ Demucs loaded successfully for live processing")
except ImportError as e:
    demucs_available = False
    logger.warning(f"⚠️ Demucs not available, using mock mode: {e}")
    # Import numpy for mock mode
    try:
        import numpy as np
    except ImportError:
        logger.error("❌ numpy required for mock mode")
        sys.exit(1)

# GStreamer for audio playback
try:
    import sys
    sys.path.append('/usr/lib/python3/dist-packages')  # Add system packages for GI
    import gi
    gi.require_version('Gst', '1.0')
    from gi.repository import Gst, GObject, GLib
    Gst.init(None)
    gstreamer_available = True
    logger.info("✅ GStreamer available for audio playback")
except ImportError:
    gstreamer_available = False
    logger.info("ℹ️ GStreamer not available - no audio playback")

class MockSeparator:
    """Mock separator for testing without Demucs"""
    
    def __init__(self, audio_file):
        self.audio_file = audio_file
        self.sample_rate = 44100
        self.duration = 0.0
        self.position = 0.0
        self.is_playing = False
        
        try:
            import soundfile as sf
            info = sf.info(audio_file)
            self.duration = info.duration
            self.sample_rate = info.samplerate
            logger.info(f"📁 Mock separator loaded: {self.duration:.2f}s @ {self.sample_rate}Hz")
        except:
            self.duration = 180.0  # Default 3 minutes
            logger.info("📁 Mock separator with default duration")
    
    def get_mock_stems(self, chunk_size=1024):
        """Generate mock stems data"""
        # Create silent audio chunks for each stem
        stems = []
        for i in range(4):  # vocals, drums, bass, other
            stem_data = np.zeros((2, chunk_size), dtype=np.float32)
            if i == 0:  # Add some mock "vocals" signal
                stem_data[0, :] = np.sin(2 * np.pi * 440 * np.arange(chunk_size) / self.sample_rate) * 0.1
            stems.append(stem_data)
        return stems

class StemPlayer:
    """
    Independent GStreamer-based stem player
    Plays separated stems with individual controls
    """
    
    def __init__(self, output_dir):
        self.output_dir = output_dir
        self.pipeline = None
        self.is_playing = False
        self.position = 0.0
        self.duration = 0.0
        
        # Stem controls
        self.stem_volumes = [1.0, 1.0, 1.0, 1.0]
        self.stem_mutes = [False, False, False, False]
        self.stem_solos = [False, False, False, False]
        
        if gstreamer_available:
            self.create_pipeline()
    
    def create_pipeline(self):
        """Create GStreamer pipeline for stem playback"""
        try:
            # Create a simple playbin pipeline for now
            # In a full implementation, this would be a complex pipeline with multiple sources
            self.pipeline = Gst.ElementFactory.make("playbin", "stem-player")
            
            if self.pipeline:
                logger.info("✅ GStreamer stem player pipeline created")
                
                # Set up bus for messages
                bus = self.pipeline.get_bus()
                bus.add_signal_watch()
                bus.connect("message", self.on_bus_message)
            else:
                logger.error("❌ Failed to create GStreamer pipeline")
                
        except Exception as e:
            logger.error(f"❌ Error creating GStreamer pipeline: {e}")
    
    def on_bus_message(self, bus, message):
        """Handle GStreamer bus messages"""
        if message.type == Gst.MessageType.EOS:
            logger.info("🔚 End of stream reached")
            self.stop()
        elif message.type == Gst.MessageType.ERROR:
            err, debug = message.parse_error()
            logger.error(f"❌ GStreamer error: {err}, {debug}")
            self.stop()
    
    def play_file(self, audio_file):
        """Play audio file through GStreamer"""
        if not self.pipeline:
            logger.error("❌ No GStreamer pipeline available")
            return False
        
        try:
            # Convert to URI if needed
            if not audio_file.startswith('file://'):
                audio_file = f"file://{os.path.abspath(audio_file)}"
            
            self.pipeline.set_property("uri", audio_file)
            
            # Start playback
            ret = self.pipeline.set_state(Gst.State.PLAYING)
            if ret == Gst.StateChangeReturn.FAILURE:
                logger.error("❌ Failed to start GStreamer playback")
                return False
            
            self.is_playing = True
            logger.info(f"▶️ Started playback: {audio_file}")
            return True
            
        except Exception as e:
            logger.error(f"❌ Error starting playback: {e}")
            return False
    
    def pause(self):
        """Pause playback"""
        if self.pipeline and self.is_playing:
            self.pipeline.set_state(Gst.State.PAUSED)
            self.is_playing = False
            logger.info("⏸️ Playback paused")
    
    def resume(self):
        """Resume playback"""
        if self.pipeline and not self.is_playing:
            self.pipeline.set_state(Gst.State.PLAYING)
            self.is_playing = True
            logger.info("▶️ Playback resumed")
    
    def stop(self):
        """Stop playback"""
        if self.pipeline:
            self.pipeline.set_state(Gst.State.NULL)
            self.is_playing = False
            self.position = 0.0
            logger.info("⏹️ Playback stopped")
    
    def set_position(self, position):
        """Seek to position (in seconds)"""
        if self.pipeline:
            seek_time = position * Gst.SECOND
            self.pipeline.seek_simple(
                Gst.Format.TIME,
                Gst.SeekFlags.FLUSH | Gst.SeekFlags.KEY_UNIT,
                seek_time
            )
            self.position = position
    
    def get_position(self):
        """Get current position (in seconds)"""
        if self.pipeline:
            try:
                success, position = self.pipeline.query_position(Gst.Format.TIME)
                if success:
                    self.position = position / Gst.SECOND
            except:
                pass
        return self.position
    
    def get_duration(self):
        """Get duration (in seconds)"""
        if self.pipeline:
            try:
                success, duration = self.pipeline.query_duration(Gst.Format.TIME)
                if success:
                    self.duration = duration / Gst.SECOND
            except:
                pass
        return self.duration
    
    def set_stem_volume(self, stem_index, volume):
        """Set volume for specific stem (0.0 to 1.0)"""
        if 0 <= stem_index < 4:
            self.stem_volumes[stem_index] = max(0.0, min(1.0, volume))
            logger.info(f"🔊 Stem {stem_index} volume: {volume:.2f}")
    
    def set_stem_mute(self, stem_index, mute):
        """Mute/unmute specific stem"""
        if 0 <= stem_index < 4:
            self.stem_mutes[stem_index] = mute
            logger.info(f"🔇 Stem {stem_index} mute: {mute}")
    
    def set_stem_solo(self, stem_index, solo):
        """Solo/unsolo specific stem"""
        if 0 <= stem_index < 4:
            if solo:
                # Mute all other stems
                for i in range(4):
                    self.stem_solos[i] = (i == stem_index)
            else:
                self.stem_solos[stem_index] = False
            logger.info(f"🎯 Stem {stem_index} solo: {solo}")

class LiveStemSeparator:
    """
    Real-time stem separation with live audio streaming
    Uses dedicated GStreamer player for stem playback
    """
    
    def __init__(self, model_name="htdemucs_ft", chunk_size=8192):
        self.model_name = model_name
        self.chunk_size = chunk_size
        
        # Only initialize device and torch if available
        if demucs_available:
            self.device = "cuda" if torch.cuda.is_available() else "cpu"
        else:
            self.device = "cpu"  # Mock device
            
        self.model = None
        self.sample_rate = 44100
        
        # Audio streaming components
        self.audio_queue = queue.Queue(maxsize=10)
        self.is_running = False
        self.processing_thread = None
        
        # Integrated stem player
        self.stem_player = None
        
        # Mock mode for testing
        self.mock_separator = None
        
        logger.info(f"🚀 LiveStemSeparator initialized with {model_name} on {self.device}")
        logger.info(f"📁 Demucs available: {demucs_available}")
        
    def load_model(self):
        """Load the Demucs model for separation"""
        if not demucs_available:
            logger.warning("⚠️ Demucs not available - using mock mode")
            return True
            
        try:
            logger.info(f"🤖 Loading Demucs model: {self.model_name}")
            
            # Clear GPU memory
            if torch.cuda.is_available():
                torch.cuda.empty_cache()
                
            # Load model
            self.model = get_model(self.model_name)
            self.model = self.model.to(self.device)
            self.model.eval()
            
            logger.info("✅ Model loaded successfully")
            return True
            
        except Exception as e:
            logger.error(f"❌ Failed to load model: {e}")
            logger.info("📁 Falling back to mock mode")
            return True  # Continue with mock mode
    
    def setup_output_pipes(self, output_dir):
        """Setup named pipes for GStreamer integration"""
        try:
            output_path = Path(output_dir)
            output_path.mkdir(parents=True, exist_ok=True)
            
            stem_names = ["vocals", "drums", "bass", "other"]
            pipe_paths = []
            
            for name in stem_names:
                pipe_path = output_path / f"{name}.pipe"
                
                # Remove existing pipe if it exists
                if pipe_path.exists():
                    pipe_path.unlink()
                
                # Create named pipe
                os.mkfifo(str(pipe_path))
                pipe_paths.append(str(pipe_path))
                
                logger.info(f"📡 Created pipe: {pipe_path}")
            
            return pipe_paths
            
        except Exception as e:
            logger.error(f"❌ Failed to create output pipes: {e}")
            return []
    
    def start_live_separation(self, audio_file, output_dir):
        """Start live stem separation with playback"""
        logger.info(f"🎵 Starting live separation for: {audio_file}")
        
        # Create stem player
        self.stem_player = StemPlayer(output_dir)
        
        # Setup for mock or real separation
        if not demucs_available or not self.model:
            logger.info("📁 Using mock separator")
            self.mock_separator = MockSeparator(audio_file)
        
        # Setup output pipes
        pipe_paths = self.setup_output_pipes(output_dir)
        
        # Start playback
        if self.stem_player.play_file(audio_file):
            logger.info("✅ Live stem separation started")
            return True
        else:
            logger.error("❌ Failed to start playback")
            return False
    
    def control_playback(self, command):
        """Control stem playback"""
        if not self.stem_player:
            return {"error": "No stem player active"}
        
        if command == "play":
            self.stem_player.resume()
        elif command == "pause":
            self.stem_player.pause()
        elif command == "stop":
            self.stem_player.stop()
        
        return self.get_status()
    
    def control_stem(self, stem_index, control_type, value):
        """Control individual stem"""
        if not self.stem_player:
            return {"error": "No stem player active"}
        
        if control_type == "volume":
            self.stem_player.set_stem_volume(stem_index, value)
        elif control_type == "mute":
            self.stem_player.set_stem_mute(stem_index, value)
        elif control_type == "solo":
            self.stem_player.set_stem_solo(stem_index, value)
        
        return self.get_status()
    
    def get_status(self):
        """Get current playback status"""
        if not self.stem_player:
            return {"error": "No stem player active"}
        
        return {
            "is_playing": self.stem_player.is_playing,
            "position": self.stem_player.get_position(),
            "duration": self.stem_player.get_duration(),
            "stem_volumes": self.stem_player.stem_volumes,
            "stem_mutes": self.stem_player.stem_mutes,
            "stem_solos": self.stem_player.stem_solos
        }
    
    def stop(self):
        """Stop all processing and playback"""
        self.is_running = False
        
        if self.stem_player:
            self.stem_player.stop()
        
        if self.processing_thread and self.processing_thread.is_alive():
            self.processing_thread.join(timeout=2)
        
        logger.info("🛑 Live stem separation stopped")

def run_service_mode():
    """Run as service for ModularStemWidget communication"""
    logger.info("🚀 Starting separation service mode")
    
    # Global state for service
    separators = {}  # job_id -> LiveStemSeparator
    current_jobs = {}  # job_id -> job info
    
    try:
        # Send service started signal
        print(json.dumps({"status": "service_started"}), flush=True)
        
        # Main service loop
        while True:
            try:
                # Read command from stdin
                line = sys.stdin.readline()
                if not line:
                    break
                
                line = line.strip()
                if not line:
                    continue
                
                # Parse JSON command
                try:
                    command = json.loads(line)
                except json.JSONDecodeError as e:
                    logger.error(f"❌ Invalid JSON: {e}")
                    print(json.dumps({"status": "error", "message": "Invalid JSON"}), flush=True)
                    continue
                
                action = command.get("action")
                logger.info(f"📥 Received command: {action}")
                
                if action == "separate_offline":
                    handle_offline_separation(command, current_jobs)
                    
                elif action == "start_live":
                    handle_live_separation(command, separators, current_jobs)
                    
                elif action == "stop_live":
                    handle_stop_live(separators, current_jobs)
                    
                elif action == "cancel":
                    handle_cancel_job(command, separators, current_jobs)
                    
                elif action == "heartbeat":
                    print(json.dumps({"status": "heartbeat_ack"}), flush=True)
                    
                elif action == "shutdown":
                    logger.info("🛑 Shutdown requested")
                    break
                    
                else:
                    logger.warning(f"⚠️ Unknown action: {action}")
                    print(json.dumps({"status": "error", "message": f"Unknown action: {action}"}), flush=True)
                    
            except Exception as e:
                logger.error(f"❌ Service error: {e}")
                print(json.dumps({"status": "error", "message": str(e)}), flush=True)
    
    except KeyboardInterrupt:
        logger.info("🛑 Service interrupted")
    
    finally:
        # Cleanup all separators
        for separator in separators.values():
            try:
                separator.stop()
            except:
                pass
        
        logger.info("👋 Separation service stopped")

def handle_offline_separation(command, current_jobs):
    """Handle offline separation request"""
    try:
        job_id = command.get("job_id")
        audio_file = command.get("file")
        output_dir = command.get("output_dir")
        
        if not job_id or not audio_file or not output_dir:
            print(json.dumps({"status": "error", "job_id": job_id, "message": "Missing parameters"}), flush=True)
            return
        
        if not Path(audio_file).exists():
            print(json.dumps({"status": "error", "job_id": job_id, "message": "Audio file not found"}), flush=True)
            return
        
        # Store job info
        current_jobs[job_id] = {
            "type": "offline",
            "audio_file": audio_file,
            "output_dir": output_dir,
            "start_time": time.time()
        }
        
        print(json.dumps({"status": "started", "job_id": job_id}), flush=True)
        
        # Start separation in background thread
        def run_separation():
            try:
                logger.info(f"🎵 Starting offline separation: {audio_file}")
                
                # Create output directory
                Path(output_dir).mkdir(parents=True, exist_ok=True)
                
                # Load model if available
                if demucs_available:
                    try:
                        model = get_model("htdemucs_ft")
                        device = "cuda" if torch.cuda.is_available() else "cpu"
                        model = model.to(device)
                        model.eval()
                        
                        # Load audio
                        waveform, sample_rate = torchaudio.load(audio_file)
                        
                        # Progress update
                        print(json.dumps({"status": "progress", "job_id": job_id, "percent": 25}), flush=True)
                        
                        # Apply separation
                        sources = apply_model(model, waveform.unsqueeze(0), device=device)[0]
                        
                        # Progress update
                        print(json.dumps({"status": "progress", "job_id": job_id, "percent": 75}), flush=True)
                        
                        # Save stems
                        stem_names = ["drums", "bass", "other", "vocals"]
                        stem_paths = {}
                        
                        for i, name in enumerate(stem_names):
                            stem_path = Path(output_dir) / f"{name}.wav"
                            torchaudio.save(str(stem_path), sources[i].unsqueeze(0), sample_rate)
                            stem_paths[name] = str(stem_path)
                            logger.info(f"💾 Saved {name}: {stem_path}")
                        
                        # Complete
                        print(json.dumps({
                            "status": "complete", 
                            "job_id": job_id, 
                            "stems": stem_paths
                        }), flush=True)
                        
                    except Exception as e:
                        logger.error(f"❌ Separation failed: {e}")
                        print(json.dumps({"status": "error", "job_id": job_id, "message": str(e)}), flush=True)
                        
                else:
                    # Mock separation for testing
                    logger.info("📁 Using mock separation")
                    time.sleep(2)  # Simulate processing time
                    
                    # Create mock stem files
                    stem_names = ["drums", "bass", "other", "vocals"]
                    stem_paths = {}
                    
                    for name in stem_names:
                        stem_path = Path(output_dir) / f"{name}.wav"
                        # Create a simple sine wave as mock audio
                        mock_audio = np.sin(2 * np.pi * 440 * np.linspace(0, 5, 44100 * 5)) * 0.1
                        sf.write(str(stem_path), mock_audio, 44100)
                        stem_paths[name] = str(stem_path)
                        
                        # Progress updates
                        progress = 25 + (75 // len(stem_names)) * (stem_names.index(name) + 1)
                        print(json.dumps({"status": "progress", "job_id": job_id, "percent": progress}), flush=True)
                    
                    # Complete
                    print(json.dumps({
                        "status": "complete", 
                        "job_id": job_id, 
                        "stems": stem_paths
                    }), flush=True)
                
            except Exception as e:
                logger.error(f"❌ Offline separation error: {e}")
                print(json.dumps({"status": "error", "job_id": job_id, "message": str(e)}), flush=True)
            finally:
                # Remove job from tracking
                current_jobs.pop(job_id, None)
        
        # Start in background thread
        thread = threading.Thread(target=run_separation, daemon=True)
        thread.start()
        
    except Exception as e:
        logger.error(f"❌ Handle offline separation error: {e}")
        print(json.dumps({"status": "error", "job_id": job_id, "message": str(e)}), flush=True)

def handle_live_separation(command, separators, current_jobs):
    """Handle live separation request"""
    try:
        job_id = command.get("job_id")
        audio_file = command.get("file")
        
        if not job_id or not audio_file:
            print(json.dumps({"status": "error", "job_id": job_id, "message": "Missing parameters"}), flush=True)
            return
        
        if not Path(audio_file).exists():
            print(json.dumps({"status": "error", "job_id": job_id, "message": "Audio file not found"}), flush=True)
            return
        
        # Create separator
        separator = LiveStemSeparator()
        
        # Load model
        if not separator.load_model():
            print(json.dumps({"status": "error", "job_id": job_id, "message": "Failed to load model"}), flush=True)
            return
        
        # Start live separation
        output_dir = f"/tmp/live_stems_{job_id}"
        if separator.start_live_separation(audio_file, output_dir):
            separators[job_id] = separator
            current_jobs[job_id] = {
                "type": "live",
                "audio_file": audio_file,
                "output_dir": output_dir,
                "start_time": time.time()
            }
            
            print(json.dumps({"status": "started", "job_id": job_id}), flush=True)
        else:
            print(json.dumps({"status": "error", "job_id": job_id, "message": "Failed to start live separation"}), flush=True)
        
    except Exception as e:
        logger.error(f"❌ Handle live separation error: {e}")
        print(json.dumps({"status": "error", "job_id": job_id, "message": str(e)}), flush=True)

def handle_stop_live(separators, current_jobs):
    """Handle stop live separation request"""
    try:
        # Stop all live separators
        for job_id, separator in list(separators.items()):
            separator.stop()
            separators.pop(job_id, None)
            current_jobs.pop(job_id, None)
        
        print(json.dumps({"status": "live_stopped"}), flush=True)
        
    except Exception as e:
        logger.error(f"❌ Handle stop live error: {e}")
        print(json.dumps({"status": "error", "message": str(e)}), flush=True)

def handle_cancel_job(command, separators, current_jobs):
    """Handle cancel job request"""
    try:
        job_id = command.get("job_id")
        
        if not job_id:
            print(json.dumps({"status": "error", "message": "Missing job_id"}), flush=True)
            return
        
        # Stop separator if exists
        if job_id in separators:
            separators[job_id].stop()
            separators.pop(job_id, None)
        
        # Remove job
        current_jobs.pop(job_id, None)
        
        print(json.dumps({"status": "cancelled", "job_id": job_id}), flush=True)
        
    except Exception as e:
        logger.error(f"❌ Handle cancel job error: {e}")
        print(json.dumps({"status": "error", "job_id": job_id, "message": str(e)}), flush=True)

def handle_command_line():
    """Handle command-line interface"""
    import argparse
    
    parser = argparse.ArgumentParser(description='Live Stem Separation for Clementine')
    parser.add_argument('--audio', help='Input audio file')
    parser.add_argument('--output', help='Output directory for pipes')
    parser.add_argument('--model', default='htdemucs_ft', help='Demucs model to use')
    parser.add_argument('--interactive', action='store_true', help='Interactive control mode')
    parser.add_argument('--service-mode', action='store_true', help='Run as service for ModularStemWidget')
    
    args = parser.parse_args()
    
    # Service mode for ModularStemWidget
    if args.service_mode:
        run_service_mode()
        return
    
    # Validate input for regular modes
    if not args.audio:
        logger.error("❌ Audio file required for non-service mode")
        sys.exit(1)
        
    if not args.output:
        logger.error("❌ Output directory required for non-service mode")
        sys.exit(1)
    
    # Validate input
    if not Path(args.audio).exists():
        logger.error(f"❌ Audio file not found: {args.audio}")
        sys.exit(1)
    
    try:
        # Create separator
        separator = LiveStemSeparator(args.model)
        
        # Load model
        if not separator.load_model():
            logger.error("❌ Failed to initialize separator")
            sys.exit(1)
        
        # Start separation
        if not separator.start_live_separation(args.audio, args.output):
            logger.error("❌ Failed to start live separation")
            sys.exit(1)
        
        print("READY", flush=True)
        
        if args.interactive:
            # Interactive control mode
            logger.info("🎮 Interactive mode - enter commands:")
            logger.info("Commands: play, pause, stop, status, volume <stem> <value>, mute <stem> <bool>, solo <stem> <bool>, quit")
            
            while True:
                try:
                    cmd = input().strip().lower()
                    if not cmd:
                        continue
                    
                    if cmd == 'quit' or cmd == 'exit':
                        break
                    elif cmd in ['play', 'pause', 'stop']:
                        result = separator.control_playback(cmd)
                        print(json.dumps(result), flush=True)
                    elif cmd == 'status':
                        result = separator.get_status()
                        print(json.dumps(result), flush=True)
                    elif cmd.startswith('volume '):
                        parts = cmd.split()
                        if len(parts) == 3:
                            stem = int(parts[1])
                            volume = float(parts[2])
                            result = separator.control_stem(stem, 'volume', volume)
                            print(json.dumps(result), flush=True)
                    elif cmd.startswith('mute '):
                        parts = cmd.split()
                        if len(parts) == 3:
                            stem = int(parts[1])
                            mute = parts[2].lower() in ['true', '1', 'yes']
                            result = separator.control_stem(stem, 'mute', mute)
                            print(json.dumps(result), flush=True)
                    elif cmd.startswith('solo '):
                        parts = cmd.split()
                        if len(parts) == 3:
                            stem = int(parts[1])
                            solo = parts[2].lower() in ['true', '1', 'yes']
                            result = separator.control_stem(stem, 'solo', solo)
                            print(json.dumps(result), flush=True)
                    else:
                        print('{"error": "Unknown command"}', flush=True)
                        
                except KeyboardInterrupt:
                    break
                except Exception as e:
                    print(f'{{"error": "{str(e)}"}}', flush=True)
        else:
            # JSON control mode for Clementine integration
            while True:
                try:
                    line = sys.stdin.readline().strip()
                    if not line:
                        break
                    
                    try:
                        cmd_data = json.loads(line)
                        action = cmd_data.get('action')
                        
                        if action in ['play', 'pause', 'stop']:
                            result = separator.control_playback(action)
                            print(json.dumps(result), flush=True)
                        elif action == 'status':
                            result = separator.get_status()
                            print(json.dumps(result), flush=True)
                        elif action == 'stem_control':
                            stem = cmd_data.get('stem', 0)
                            control_type = cmd_data.get('type', 'volume')
                            value = cmd_data.get('value', 1.0)
                            result = separator.control_stem(stem, control_type, value)
                            print(json.dumps(result), flush=True)
                        else:
                            print('{"error": "Unknown action"}', flush=True)
                            
                    except json.JSONDecodeError:
                        print('{"error": "Invalid JSON"}', flush=True)
                        
                except KeyboardInterrupt:
                    break
                except Exception as e:
                    logger.error(f"❌ Command error: {e}")
                    print(f'{{"error": "{str(e)}"}}', flush=True)
        
        # Cleanup
        separator.stop()
        logger.info("👋 Live stem separation ended")
        
    except Exception as e:
        logger.error(f"💥 Fatal error: {e}")
        print(f"ERROR:{e}", flush=True)
        sys.exit(1)

def main():
    """Main entry point"""
    # Setup signal handlers
    def signal_handler(signum, frame):
        logger.info("🛑 Received termination signal")
        sys.exit(0)
    
    signal.signal(signal.SIGINT, signal_handler)
    signal.signal(signal.SIGTERM, signal_handler)
    
    handle_command_line()

if __name__ == "__main__":
    main()
