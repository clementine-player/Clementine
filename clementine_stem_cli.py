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
Command-line interface for Clementine's AI Stem Separation
Compatible with C++ QProcess execution
"""

import argparse
import os
import sys
import logging

# Force unbuffered output for QProcess communication
sys.stdout.reconfigure(line_buffering=True)
sys.stderr.reconfigure(line_buffering=True)
from pathlib import Path

# Set up logging to help with C++ debugging
logging.basicConfig(
    level=logging.INFO,
    format='%(asctime)s - %(levelname)s - %(message)s',
    handlers=[
        logging.StreamHandler(sys.stdout),
        logging.FileHandler('stem_separation.log', mode='a')
    ]
)
logger = logging.getLogger(__name__)

try:
    from demucs import separate
    from demucs.pretrained import get_model
    from demucs.apply import apply_model
    import torch
    import torchaudio
    import soundfile as sf
    import numpy as np
    demucs_available = True
    logger.info("✅ Demucs loaded successfully")
except ImportError as e:
    demucs_available = False
    logger.error(f"❌ Failed to import Demucs: {e}")
    
try:
    import torch
    torch_available = torch.cuda.is_available()
    if torch_available:
        logger.info(f"🚀 CUDA available with {torch.cuda.device_count()} GPU(s)")
    else:
        logger.info("⚠️ CUDA not available, using CPU")
except ImportError:
    torch_available = False
    logger.warning("⚠️ PyTorch not available")

class ClementineStemSeparator:
    """
    Advanced stem separator for Clementine Extended
    Command-line compatible version
    """
    
    def __init__(self, model_name="htdemucs"):
        self.model_name = model_name
        self.model = None
        self.device = "cuda" if torch.cuda.is_available() else "cpu"
        logger.info(f"🎯 Initializing with model: {model_name} on device: {self.device}")
        
    def load_model(self):
        """Load the Demucs model"""
        if not demucs_available:
            raise RuntimeError("Demucs not available")
            
        try:
            logger.info(f"📦 Loading model: {self.model_name}...")
            self.model = get_model(self.model_name)
            self.model.to(self.device)
            logger.info(f"✅ Model loaded successfully on {self.device}")
            return True
        except Exception as e:
            logger.error(f"❌ Failed to load model: {e}")
            raise
    
    def separate_audio(self, audio_file, output_dir):
        """
        Separate audio into stems
        
        Args:
            audio_file (str): Path to input audio file
            output_dir (str): Directory to save separated stems
            
        Returns:
            dict: Paths to separated stem files
        """
        if not self.model:
            self.load_model()
            
        audio_path = Path(audio_file)
        output_path = Path(output_dir)
        output_path.mkdir(parents=True, exist_ok=True)
        
        logger.info(f"🎵 Processing: {audio_path.name}")
        logger.info(f"📂 Output directory: {output_path}")
        
        try:
            # Progress indicator for C++
            print("PROGRESS:10", flush=True)
            
            # Load audio
            logger.info("🎵 Loading audio file...")
            audio, sr = torchaudio.load(str(audio_path))
            
            # Ensure stereo
            if audio.shape[0] == 1:
                audio = audio.repeat(2, 1)
            elif audio.shape[0] > 2:
                audio = audio[:2]
            
            # Move to device
            audio = audio.to(self.device)
            
            print("PROGRESS:30", flush=True)
            
            # Apply separation using proper model
            logger.info("🔄 Starting stem separation...")
            with torch.no_grad():
                stems = apply_model(self.model, audio.unsqueeze(0))
            
            print("PROGRESS:70", flush=True)
            
            # Save separated stems
            stems = stems.squeeze(0).cpu().numpy()
            stem_files = {}
            stem_names = ['drums', 'bass', 'other', 'vocals']
            
            logger.info("💾 Saving separated stems...")
            
            for i, stem_name in enumerate(stem_names):
                if i < stems.shape[0]:
                    stem_filename = f"{audio_path.stem}_{stem_name}.wav"
                    stem_path = output_path / stem_filename
                    
                    # Get stem audio and ensure correct shape
                    stem_audio = stems[i]
                    if len(stem_audio.shape) == 1:
                        stem_audio = np.stack([stem_audio, stem_audio])
                    
                    # Save stem using soundfile for better compatibility
                    sf.write(str(stem_path), stem_audio.T, sr)
                    
                    stem_files[stem_name] = str(stem_path)
                    logger.info(f"✅ Saved {stem_name}: {stem_path.name}")
                    
                    print(f"PROGRESS:{75 + i*5}", flush=True)
            
            print("PROGRESS:100", flush=True)
            logger.info("🎉 Stem separation completed successfully!")
            
            return stem_files
            
        except Exception as e:
            logger.error(f"❌ Separation failed: {e}")
            import traceback
            traceback.print_exc()
            raise

def main():
    parser = argparse.ArgumentParser(description='AI Stem Separation for Clementine Extended')
    parser.add_argument('--audio', required=True, help='Input audio file path')
    parser.add_argument('--output', required=True, help='Output directory for stems')
    parser.add_argument('--model', default='htdemucs', help='Demucs model to use')
    
    args = parser.parse_args()
    
    # Validate input file
    if not Path(args.audio).exists():
        logger.error(f"❌ Audio file not found: {args.audio}")
        sys.exit(1)
    
    if not demucs_available:
        logger.error("❌ Demucs not available. Please install with: pip install demucs")
        sys.exit(1)
    
    try:
        # Create separator and process
        separator = ClementineStemSeparator(args.model)
        stem_files = separator.separate_audio(args.audio, args.output)
        
        # Output results for C++ parsing
        print("SEPARATION_COMPLETE", flush=True)
        for stem_name, file_path in stem_files.items():
            print(f"STEM:{stem_name}:{file_path}", flush=True)
        
        logger.info("🎯 Command completed successfully")
        sys.exit(0)
        
    except Exception as e:
        logger.error(f"💥 Fatal error: {e}")
        print(f"ERROR:{e}", flush=True)
        sys.exit(1)

if __name__ == "__main__":
    main()
