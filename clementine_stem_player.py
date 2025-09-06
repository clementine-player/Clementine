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
"""

import os
import sys
import time
import threading
import logging
import queue
import signal
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
    logger.error(f"❌ Failed to import required libraries: {e}")

# PyAudio is optional - only needed for actual audio device streaming
try:
    import pyaudio
    pyaudio_available = True
    logger.info("✅ PyAudio available for live audio streaming")
except ImportError:
    pyaudio_available = False
    logger.info("ℹ️ PyAudio not available - using file-based streaming")

class LiveStemSeparator:
    """
    Controlled stem player with play/pause/stop functionality
    Separates audio and provides individual stem control
    """
    
    def __init__(self, model_name="htdemucs_ft"):
        self.model_name = model_name
        self.device = "cuda" if torch.cuda.is_available() else "cpu"
        self.model = None
        self.sample_rate = 44100
        
        # Playback state
        self.state = PlaybackState.STOPPED
        self.current_audio_file = None
        self.separated_stems = None
        self.playback_position = 0.0
        self.total_duration = 0.0
        
        # Audio data
        self.original_audio = None
        self.stems_data = [None, None, None, None]  # vocals, drums, bass, other
        
        # Volume controls (0.0 to 1.0)
        self.stem_volumes = [1.0, 1.0, 1.0, 1.0]
        self.stem_mutes = [False, False, False, False]
        self.stem_solos = [False, False, False, False]
        
        # Control threads
        self.playback_thread = None
        self.control_thread = None
        self.is_running = False
        
        # Named pipes for output
        self.output_pipes = []
        
        logger.info(f"🎵 StemPlayer initialized with {model_name} on {self.device}")
        
    def load_model(self):
        """Load the Demucs model for stem separation"""
        if not demucs_available:
            logger.error("❌ Demucs not available")
            return False
            
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
            return False
    
    def load_audio(self, audio_file):
        """Load and separate audio file into stems"""
        try:
            logger.info(f"🎵 Loading audio: {audio_file}")
            
            # Load audio file
            waveform, sr = torchaudio.load(audio_file)
            
            # Resample if needed
            if sr != self.sample_rate:
                resampler = torchaudio.transforms.Resample(sr, self.sample_rate)
                waveform = resampler(waveform)
            
            # Convert to stereo if mono
            if waveform.shape[0] == 1:
                waveform = waveform.repeat(2, 1)
            elif waveform.shape[0] > 2:
                waveform = waveform[:2]  # Take first 2 channels
            
            self.original_audio = waveform
            self.total_duration = waveform.shape[1] / self.sample_rate
            self.current_audio_file = audio_file
            
            logger.info(f"✅ Audio loaded: {self.total_duration:.2f}s, {waveform.shape}")
            
            # Separate stems if model is loaded
            if self.model is not None:
                return self.separate_stems()
            else:
                logger.warning("⚠️ Model not loaded - cannot separate stems")
                return False
                
        except Exception as e:
            logger.error(f"❌ Failed to load audio: {e}")
            return False
    
    def separate_stems(self):
        """Separate the loaded audio into stems"""
        if self.original_audio is None or self.model is None:
            logger.error("❌ Audio or model not loaded")
            return False
            
        try:
            logger.info("🤖 Separating stems...")
            
            # Move audio to device
            audio_tensor = self.original_audio.unsqueeze(0).to(self.device)
            
            # Separate stems
            with torch.no_grad():
                stems = apply_model(self.model, audio_tensor, device=self.device, progress=True)
            
            # Convert to numpy and store
            stems = stems.squeeze(0).cpu().numpy()
            
            # Store stems (vocals, drums, bass, other)
            for i in range(4):
                if i < stems.shape[0]:
                    self.stems_data[i] = stems[i]
                else:
                    # Fallback to silence if stem not available
                    self.stems_data[i] = np.zeros((2, self.original_audio.shape[1]))
            
            logger.info("✅ Stems separated successfully")
            return True
            
        except Exception as e:
            logger.error(f"❌ Failed to separate stems: {e}")
            return False
    
    def create_output_pipes(self, output_dir):
        """Create named pipes for each stem"""
        try:
            output_path = Path(output_dir)
            output_path.mkdir(parents=True, exist_ok=True)
            
            stem_names = ["vocals", "drums", "bass", "other"]
            
            for i, name in enumerate(stem_names):
                pipe_path = output_path / f"{name}.pipe"
                
                # Remove existing pipe if it exists
                if pipe_path.exists():
                    pipe_path.unlink()
                
                # Create named pipe
                os.mkfifo(str(pipe_path))
                self.output_pipes.append(str(pipe_path))
                
                logger.info(f"📡 Created pipe: {pipe_path}")
            
            return True
            
        except Exception as e:
            logger.error(f"❌ Failed to create output pipes: {e}")
            return False
    
    def play(self):
        """Start playback"""
        if self.stems_data[0] is None:
            logger.error("❌ No stems available for playback")
            return False
        
        if self.state == PlaybackState.PLAYING:
            logger.info("ℹ️ Already playing")
            return True
        
        logger.info("▶️ Starting playback")
        self.state = PlaybackState.PLAYING
        
        if self.playback_thread is None or not self.playback_thread.is_alive():
            self.is_running = True
            self.playback_thread = threading.Thread(target=self._playback_loop)
            self.playback_thread.start()
        
        return True
    
    def pause(self):
        """Pause playback"""
        if self.state == PlaybackState.PLAYING:
            logger.info("⏸️ Pausing playback")
            self.state = PlaybackState.PAUSED
            return True
        return False
    
    def resume(self):
        """Resume playback"""
        if self.state == PlaybackState.PAUSED:
            logger.info("▶️ Resuming playback")
            self.state = PlaybackState.PLAYING
            return True
        return False
    
    def stop(self):
        """Stop playback"""
        logger.info("⏹️ Stopping playback")
        self.state = PlaybackState.STOPPED
        self.playback_position = 0.0
        return True
    
    def set_stem_volume(self, stem_index, volume):
        """Set volume for a specific stem (0.0 to 1.0)"""
        if 0 <= stem_index < 4:
            self.stem_volumes[stem_index] = max(0.0, min(1.0, volume))
            logger.info(f"🔊 Stem {stem_index} volume: {self.stem_volumes[stem_index]:.2f}")
            return True
        return False
    
    def set_stem_mute(self, stem_index, mute):
        """Mute/unmute a specific stem"""
        if 0 <= stem_index < 4:
            self.stem_mutes[stem_index] = mute
            logger.info(f"🔇 Stem {stem_index} mute: {mute}")
            return True
        return False
    
    def set_stem_solo(self, stem_index, solo):
        """Solo/unsolo a specific stem"""
        if 0 <= stem_index < 4:
            # If soloing, unmute this stem and mute others
            if solo:
                for i in range(4):
                    self.stem_solos[i] = (i == stem_index)
            else:
                self.stem_solos[stem_index] = False
            
            logger.info(f"🎯 Stem {stem_index} solo: {solo}")
            return True
        return False
    
    def _playback_loop(self):
        """Main playback loop"""
        chunk_size = 1024
        chunk_duration = chunk_size / self.sample_rate
        
        while self.is_running and self.state != PlaybackState.STOPPED:
            if self.state == PlaybackState.PLAYING:
                # Calculate current chunk
                start_sample = int(self.playback_position * self.sample_rate)
                end_sample = start_sample + chunk_size
                
                # Check if we've reached the end
                if start_sample >= self.original_audio.shape[1]:
                    self.stop()
                    break
                
                # Mix stems based on volume/mute/solo settings
                mixed_chunk = self._mix_stems_chunk(start_sample, end_sample)
                
                # Write to output pipes (if they exist)
                self._write_to_pipes(mixed_chunk)
                
                # Update position
                self.playback_position += chunk_duration
                
                # Sleep to maintain real-time playback
                time.sleep(chunk_duration)
            
            elif self.state == PlaybackState.PAUSED:
                # Just wait while paused
                time.sleep(0.1)
    
    def _mix_stems_chunk(self, start_sample, end_sample):
        """Mix stems according to current volume/mute/solo settings"""
        # Initialize output
        output_chunk = np.zeros((2, end_sample - start_sample))
        
        # Check if any stem is soloed
        any_solo = any(self.stem_solos)
        
        for i in range(4):
            if self.stems_data[i] is None:
                continue
            
            # Get chunk from this stem
            stem_chunk = self.stems_data[i][:, start_sample:end_sample]
            
            # Apply volume, mute, and solo logic
            should_play = True
            
            if any_solo:
                should_play = self.stem_solos[i]
            elif self.stem_mutes[i]:
                should_play = False
            
            if should_play:
                # Apply volume
                stem_chunk *= self.stem_volumes[i]
                output_chunk += stem_chunk
        
        return output_chunk
    
    def _write_to_pipes(self, audio_chunk):
        """Write audio chunk to output pipes"""
        # For now, just log that we would write to pipes
        # In a real implementation, this would write to named pipes
        pass
    
    def get_status(self):
        """Get current playback status"""
        return {
            "state": self.state.name,
            "position": self.playback_position,
            "duration": self.total_duration,
            "progress": self.playback_position / max(self.total_duration, 1.0),
            "stem_volumes": self.stem_volumes,
            "stem_mutes": self.stem_mutes,
            "stem_solos": self.stem_solos
        }
    
    def handle_command(self, command):
        """Handle control commands from external sources"""
        try:
            cmd_data = json.loads(command) if isinstance(command, str) else command
            
            action = cmd_data.get("action")
            
            if action == "play":
                return self.play()
            elif action == "pause":
                return self.pause()
            elif action == "resume":
                return self.resume()
            elif action == "stop":
                return self.stop()
            elif action == "set_volume":
                stem = cmd_data.get("stem", 0)
                volume = cmd_data.get("volume", 1.0)
                return self.set_stem_volume(stem, volume)
            elif action == "set_mute":
                stem = cmd_data.get("stem", 0)
                mute = cmd_data.get("mute", False)
                return self.set_stem_mute(stem, mute)
            elif action == "set_solo":
                stem = cmd_data.get("stem", 0)
                solo = cmd_data.get("solo", False)
                return self.set_stem_solo(stem, solo)
            elif action == "status":
                return self.get_status()
            else:
                logger.warning(f"⚠️ Unknown command: {action}")
                return False
                
        except Exception as e:
            logger.error(f"❌ Failed to handle command: {e}")
            return False

def main():
    import argparse
    
    parser = argparse.ArgumentParser(description='Clementine Stem Player')
    parser.add_argument('--audio', required=True, help='Input audio file path')
    parser.add_argument('--output', required=True, help='Output directory for pipes')
    parser.add_argument('--model', default='htdemucs_ft', help='Demucs model to use')
    parser.add_argument('--control', help='Control command file (JSON)')
    
    args = parser.parse_args()
    
    # Validate input
    if not Path(args.audio).exists():
        logger.error(f"❌ Audio file not found: {args.audio}")
        sys.exit(1)
    
    try:
        # Create stem player
        player = StemPlayer(args.model)
        
        # Load model and audio
        if not player.load_model():
            logger.error("❌ Failed to load model")
            sys.exit(1)
        
        if not player.load_audio(args.audio):
            logger.error("❌ Failed to load audio")
            sys.exit(1)
        
        # Create output pipes
        if not player.create_output_pipes(args.output):
            logger.error("❌ Failed to create output pipes")
            sys.exit(1)
        
        logger.info("🎵 Stem player ready!")
        print("READY", flush=True)
        
        # Start control loop
        while True:
            try:
                # Read command from stdin
                line = sys.stdin.readline().strip()
                if not line:
                    break
                
                # Handle command
                result = player.handle_command(line)
                
                # Send response
                if isinstance(result, dict):
                    print(json.dumps(result), flush=True)
                else:
                    print("OK" if result else "ERROR", flush=True)
                    
            except KeyboardInterrupt:
                break
            except Exception as e:
                logger.error(f"❌ Command error: {e}")
                print("ERROR", flush=True)
        
        # Cleanup
        player.stop()
        logger.info("👋 Stem player stopped")
        
    except Exception as e:
        logger.error(f"💥 Fatal error: {e}")
        print(f"ERROR:{e}", flush=True)
        sys.exit(1)

if __name__ == "__main__":
    main()
