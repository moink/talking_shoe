import os
import shutil
os.environ["PATH"] += os.pathsep + r"C:\ffmpeg\bin"

from pydub import AudioSegment

# Set your ffmpeg/ffprobe paths if needed
AudioSegment.converter = r"C:\ffmpeg\bin\ffmpeg.exe"
AudioSegment.ffprobe = r"C:\ffmpeg\bin\ffprobe.exe"

# Paths
INPUT_DIR = r"C:\Users\Moink\Downloads\DasImprovRecordings\DasImprovRecordings"
OUTPUT_DIR = r"C:\Users\Moink\PycharmProjects\pythonProject2\mp3_output"

# Ensure output dir exists
os.makedirs(OUTPUT_DIR, exist_ok=True)

START_TRACK = 43  # Harry's first track number

# Prepare table
track_table_entries = [[] for _ in range(5)]  # 5 activities/scenarios

current_track = START_TRACK

for scenario in range(1, 6):
    scenario_dir = os.path.join(INPUT_DIR, f"response_{scenario}")
    if not os.path.isdir(scenario_dir):
        print(f"Missing folder: {scenario_dir}")
        continue

    # Sort responses for consistency
    files = sorted([f for f in os.listdir(scenario_dir) if f.lower().endswith(".mp3")])

    for file_name in files:
        input_path = os.path.join(scenario_dir, file_name)
        new_file_name = f"{current_track:04}.mp3"
        output_path = os.path.join(OUTPUT_DIR, new_file_name)

        # Copy & rename
        shutil.copy(input_path, output_path)

        # Read duration
        audio = AudioSegment.from_mp3(output_path)
        length_ms = len(audio)

        track_table_entries[scenario - 1].append((current_track, length_ms))

        print(
            f"Copied {file_name} -> {new_file_name} (Activity {scenario - 1}), {length_ms} ms")
        current_track += 1

# Print C++ track table for Harry
print("\n// Harry's track table (lengths in ms)")
print("{")
for i, tracks in enumerate(track_table_entries):
    entries_str = ", ".join(f"{{{t[0]}, {t[1]}}}" for t in tracks)
    print(f"    {{ {entries_str} }}, {len(tracks)} }},  // Activity {i}")
print("}")
