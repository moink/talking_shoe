import os
os.environ["PATH"] += os.pathsep + r"C:\ffmpeg\bin"
from pydub import AudioSegment, effects

# Paths
OUTPUT_DIR = os.path.join(os.path.dirname(__file__), "mp3_output")
NORMALIZED_OUTPUT_DIR = os.path.join(os.path.dirname(__file__), "normalized_output")

os.makedirs(NORMALIZED_OUTPUT_DIR, exist_ok=True)

def normalize_file(input_path, output_path):
    audio = AudioSegment.from_mp3(input_path)
    normalized_audio = effects.normalize(audio)
    normalized_audio.export(output_path, format="mp3")
    print(f"Normalized {os.path.basename(input_path)} -> {os.path.basename(output_path)}")

def main():
    for file_name in sorted(os.listdir(OUTPUT_DIR)):
        if file_name.lower().endswith(".mp3"):
            input_path = os.path.join(OUTPUT_DIR, file_name)
            output_path = os.path.join(NORMALIZED_OUTPUT_DIR, file_name)
            normalize_file(input_path, output_path)

if __name__ == "__main__":
    main()