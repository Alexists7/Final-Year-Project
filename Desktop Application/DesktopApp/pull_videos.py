import sys
import subprocess

def pull_files(device_ip, password):
    # Remote path with spaces and parentheses wrapped in quotes
    remote_path = f'"root@{device_ip}:/home/ashaju/Platform Test/3) Face Detection/data"'
    local_path = "."

    # Construct SCP command
    scp_command = f"sshpass -p {password} scp -r {remote_path} {local_path}"

    print(f"Running SCP command: {scp_command}")  # Debug output

    try:
        # Run SCP command
        result = subprocess.run(scp_command, shell=True, capture_output=True, text=True, check=True)

        print("SCP Transfer Successful")
        print(result.stdout)
    except subprocess.CalledProcessError as e:
        print("SCP Transfer Failed")
        print(f"Error Output: {e.stderr}")
        sys.exit(1)

if __name__ == "__main__":
    if len(sys.argv) < 3:
        print("Usage: python3 pull_videos.py <Device_IP> <Password>")
        sys.exit(1)

    device_ip = sys.argv[1]
    password = sys.argv[2]

    pull_files(device_ip, password)
