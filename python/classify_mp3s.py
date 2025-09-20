import os
os.environ["PATH"] += os.pathsep + r"C:\ffmpeg\bin"
from pydub import AudioSegment
from pydub.playback import play

# --- FFmpeg / FFprobe setup for Windows ---
FFMPEG_PATH = r"C:\ffmpeg\bin\ffmpeg.exe"
FFPROBE_PATH = r"C:\ffmpeg\bin\ffprobe.exe"

# --- CONFIG ---
INPUT_DIR = os.path.join(os.path.dirname(__file__), "mp3_input")
OUTPUT_DIR = os.path.join(os.path.dirname(__file__), "mp3_output")
CHARACTER_INDEX = 0          # character you are populating (e.g., Matt)
NUM_ACTIVITIES = 5           # 1: DOING_NOTHING ... 5: JUST_REMOVED
NUM_DIGITS = 4

def main():
    os.makedirs(OUTPUT_DIR, exist_ok=True)
    files = sorted([f for f in os.listdir(INPUT_DIR) if f.lower().endswith(".mp3")])
    file_counter = 1
    NUM_ACTIVITIES = 5
    track_table = [[] for _ in range(NUM_ACTIVITIES)]  # one list per Activity

    for original_file in files:
        path = os.path.join(INPUT_DIR, original_file)
        audio = AudioSegment.from_mp3(path)
        print(f"\nPlaying {original_file} ({len(audio)} ms)")
        play(audio)

        while True:
            choice_str = input("Enter activity (0-4) or just Enter to skip: ").strip()
            if choice_str == "":
                print("Skipping file.")
                break
            try:
                choice = int(choice_str)
                if 0 <= choice <= 4:
                    # Keep the file
                    new_name = f"{file_counter:0{NUM_DIGITS}d}.mp3"
                    new_path = os.path.join(OUTPUT_DIR, new_name)
                    audio.export(new_path, format="mp3")
                    print(f"Saved as {new_name}")
                    # Add to track_table
                    track_table[choice].append((file_counter, len(audio)))
                    file_counter += 1
                    break
            except ValueError:
                pass
            print("Invalid input, try again.")

    # Print C++ track table
    print("\nC++ Track Table:")
    print("{")
    for idx, tracks in enumerate(track_table):
        print(f"    {{ ", end="")
        for track_number, length_ms in tracks:
            print(f"{{{track_number}, {length_ms}}}, ", end="")
        print(f"}}, {len(tracks)} }},  // Activity {idx}")
    print("}")

if __name__ == "__main__":
    main()
