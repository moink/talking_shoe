import os
os.environ["PATH"] += os.pathsep + r"C:\ffmpeg\bin"
from pydub import AudioSegment

TINA_DIR = r"C:\Users\Moink\PycharmProjects\pythonProject2\tina_files"

# mapping from activity string to enum index
ACTIVITY_MAP = {
    "doing nothing": 0,
    "putting on": 1,
    "worn standing still": 2,
    "walking": 3,
    "just took off": 4
}

# track numbers and activities
TRACKS = [
    (18, "doing nothing"), (19, "doing nothing"), (20, "doing nothing"),
    (21, "doing nothing"), (22, "doing nothing"),
    (23, "putting on"), (24, "putting on"), (25, "putting on"),
    (26, "putting on"), (27, "putting on"),
    (28, "worn standing still"), (29, "worn standing still"),
    (30, "worn standing still"), (31, "worn standing still"),
    (32, "worn standing still"),
    (33, "walking"), (34, "walking"), (35, "walking"),
    (36, "walking"), (37, "walking"), (38, "walking"),
    (39, "just took off"), (40, "just took off"),
    (41, "just took off"), (42, "just took off")
]

# organize by activity
activity_tracks = {i: [] for i in range(5)}

for track_num, activity_str in TRACKS:
    activity_idx = ACTIVITY_MAP[activity_str]
    file_name = os.path.join(TINA_DIR, f"{track_num:04}.mp3")
    audio = AudioSegment.from_mp3(file_name)
    length_ms = len(audio)
    activity_tracks[activity_idx].append((track_num, length_ms))

# print as C++ StateTracks
print("    // Character 2: Tina")
print("    {")
for activity_idx in range(5):
    tracks = activity_tracks[activity_idx]
    tracks_str = ", ".join(f"{{{num}, {length}}}" for num, length in tracks)
    print(f"        // {list(ACTIVITY_MAP.keys())[activity_idx]} ({activity_idx})")
    print(f"        {{ {tracks_str} }}, {len(tracks)} }},")
print("    },")