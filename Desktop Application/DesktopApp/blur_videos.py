import cv2
import os
import imutils

# Directories
data_dir = "data"
output_dir = "processed_videos"
os.makedirs(output_dir, exist_ok=True)

# Load the pre-trained Haar Cascade classifiers for frontal and profile face detection
face_cascade = cv2.CascadeClassifier(cv2.data.haarcascades + "haarcascade_frontalface_default.xml")
profile_face_cascade = cv2.CascadeClassifier(cv2.data.haarcascades + "haarcascade_profileface.xml")

def blur_face(frame, top, right, bottom, left):
    """Applies a strong Gaussian blur to the face region."""
    face_region = frame[top:bottom, left:right]
    # Apply a stronger blur effect
    blurred_face = cv2.GaussianBlur(face_region, (99, 99), 30)
    frame[top:bottom, left:right] = blurred_face
    return frame

def process_frame(frame, scale_ratio):
    """Processes a single frame: detects faces and applies blur effect consistently."""
    resized_frame = imutils.resize(frame, width=320)  # Reduce resolution for faster detection
    gray_frame = cv2.cvtColor(resized_frame, cv2.COLOR_BGR2GRAY)  # Convert to grayscale for face detection

    # Detect frontal faces using Haar Cascade (OpenCV)
    faces = face_cascade.detectMultiScale(gray_frame, scaleFactor=1.1, minNeighbors=5, minSize=(30, 30))
    # Detect profile faces using Haar Cascade (OpenCV)
    profile_faces = profile_face_cascade.detectMultiScale(gray_frame, scaleFactor=1.1, minNeighbors=5, minSize=(30, 30))

    # Ensure both lists are not empty before concatenating
    all_faces = []
    if len(faces) > 0:
        all_faces.extend(faces)
    if len(profile_faces) > 0:
        all_faces.extend(profile_faces)

    # Check if a face is detected for the first time or needs to maintain the blur
    for (x, y, w, h) in all_faces:
        # Scale back face coordinates to original frame size
        top = int(y * scale_ratio)
        left = int(x * scale_ratio)
        bottom = int((y + h) * scale_ratio)
        right = int((x + w) * scale_ratio)

        # Apply blur effect only if the face was detected or was detected previously
        frame = blur_face(frame, top, right, bottom, left)

    # Return the processed frame
    return frame


def process_video(video_path, output_path):
    """Processes a video to detect faces and apply blur effect."""
    cap = cv2.VideoCapture(video_path)
    fourcc = cv2.VideoWriter_fourcc(*"mp4v")
    fps = int(cap.get(cv2.CAP_PROP_FPS))
    width = int(cap.get(cv2.CAP_PROP_FRAME_WIDTH))
    height = int(cap.get(cv2.CAP_PROP_FRAME_HEIGHT))
    out = cv2.VideoWriter(output_path, fourcc, fps, (width, height))

    scale_ratio = width / 320  # Scaling factor to revert face coordinates later

    while cap.isOpened():
        ret, frame = cap.read()
        if not ret:
            break

        # Process the frame
        processed_frame = process_frame(frame, scale_ratio)

        # Write the processed frame to output file
        out.write(processed_frame)

    cap.release()
    out.release()
    print(f"Processed {video_path} -> {output_path}")

# Process all MP4 files in data directory and subdirectories
for root, _, files in os.walk(data_dir):
    for filename in files:
        if filename.endswith(".mp4"):
            input_path = os.path.join(root, filename)
            relative_path = os.path.relpath(root, data_dir)
            output_subdir = os.path.join(output_dir, relative_path)
            os.makedirs(output_subdir, exist_ok=True)
            output_path = os.path.join(output_subdir, filename.replace(".mp4", "_detected.avi"))

            if os.path.exists(output_path):
                print(f"Skipping {input_path} (Already Processed)")
                continue

            process_video(input_path, output_path)

print("Processing complete!")
