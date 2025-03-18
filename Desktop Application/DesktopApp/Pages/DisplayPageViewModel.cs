using CommunityToolkit.Mvvm.ComponentModel;
using CommunityToolkit.Mvvm.Input;
using Firebase.Auth;
using System.Diagnostics;

namespace DesktopApp.Pages
{
    public partial class DisplayPageViewModel : ObservableObject
    {
        private readonly FirebaseAuthClient _authClient;

        [ObservableProperty]
        private string _deviceIpAddress;

        [ObservableProperty]
        private bool _isAdmin;

        [ObservableProperty]
        private bool _isDataFetched = false;

        [ObservableProperty]
        private List<string> _videoFiles = new List<string>();

        [ObservableProperty]
        private string _selectedVideoPath;

        [ObservableProperty]
        private bool _isVideoSelected = false;

        public string Username => _authClient.User?.Info?.DisplayName ?? "Guest";
        public string UserRole => IsAdmin ? "Admin" : "User";
        public bool IsIpEntryVisible => !IsDataFetched;
        public IRelayCommand PlayVideoCommand { get; }
        private string _selectedVideo;
        public string SelectedVideo
        {
            get => _selectedVideo;
            set
            {
                _selectedVideo = value;
                OnPropertyChanged(nameof(SelectedVideo));

                if (IsAdmin)
                {
                    SelectedVideoPath = Path.GetFullPath(Path.Combine(AppDomain.CurrentDomain.BaseDirectory, @"../../../", "data", SelectedVideo));
                }
                else
                {
                    SelectedVideoPath = Path.GetFullPath(Path.Combine(AppDomain.CurrentDomain.BaseDirectory, @"../../../", "processed_videos", SelectedVideo));
                }

                Console.WriteLine($"Selected video: {SelectedVideo}");
                Console.WriteLine($"Selected video path: {SelectedVideoPath}");
                Console.WriteLine("base dir" + AppDomain.CurrentDomain.BaseDirectory);
            }
        }

        public DisplayPageViewModel(FirebaseAuthClient authClient)
        {
            _authClient = authClient;

            // Initialize PlayVideoCommand
            PlayVideoCommand = new RelayCommand(PlayVideo, CanPlayVideo);
        }

         // Method to check if the video is selected
        private bool CanPlayVideo()
        {
            return IsVideoSelected;  // Only allow play if a video is selected
        }

        // Method to execute when the PlayVideo command is triggered
        private void PlayVideo()
        {
            if (!string.IsNullOrWhiteSpace(SelectedVideoPath))
            {
                // Logic to play the video
                Console.WriteLine($"Playing video from: {SelectedVideoPath}");

                // Add your video playback logic here
                // For example, if using a MediaElement or some other video player, set its source
            }
            else
            {
                Console.WriteLine("No video selected.");
            }
        }

        // A method to simulate video selection change (triggered by UI interaction)
        public void OnVideoSelected(string videoPath)
        {
            // Update the selected video path and set the video as selected
            SelectedVideoPath = videoPath;
            IsVideoSelected = true;
            Console.WriteLine($"Video selected: {SelectedVideoPath}");
        }

        public void OnAppearing()
        {
            OnPropertyChanged(nameof(Username));

            var user = _authClient.User;

            if (user != null)
            {
                IsAdmin = CheckWhitelist(user.Info.Email);
            }
            else
            {
                IsAdmin = false;
                IsDataFetched = false;

                OnPropertyChanged(nameof(IsDataFetched));
                OnPropertyChanged(nameof(IsIpEntryVisible));
            }

            OnPropertyChanged(nameof(UserRole));
        }

        private bool CheckWhitelist(string email)
        {
            try
            {
                // Read the whitelist file
                string filePath = Path.GetFullPath(Path.Combine(AppDomain.CurrentDomain.BaseDirectory, @"..\..\..\..\whitelist.txt"));

                if (File.Exists(filePath))
                {
                    var whitelistEmails = File.ReadAllLines(filePath);
                    return whitelistEmails.Contains(email);
                }
            }
            catch (Exception ex)
            {
                Console.WriteLine($"Error reading whitelist: {ex.Message}");
            }

            return false;
        }

        [RelayCommand]
        private async Task SaveIp()
        {
            if (!string.IsNullOrWhiteSpace(DeviceIpAddress))
            {
                Console.WriteLine($"Device IP Address saved: {DeviceIpAddress}");

                try
                {
                    // Prompt for the SCP password
                    string scpPassword = await Application.Current.MainPage.DisplayPromptAsync(
                        "RB5 Password",
                        "Enter the RB5 password below:",
                        accept: "OK",
                        cancel: "Cancel",
                        maxLength: 100,
                        keyboard: Keyboard.Text
                    );

                    if (string.IsNullOrWhiteSpace(scpPassword))
                    {
                        await Application.Current.MainPage.DisplayAlert("Error", "Password is required.", "OK");
                        return;
                    }

                    // Define the SCP command directly
                    string command = $"sshpass -p \"{scpPassword}\" scp -r \"root@{DeviceIpAddress}:/home/ashaju/Platform Test/3) Face Detection/data\" ./DesktopApp";

                    using (Process process = new Process())
                    {
                        // Start the process and run it in WSL
                        process.StartInfo.FileName = "wsl.exe";  // Use wsl.exe directly
                        process.StartInfo.Arguments = command;  // Directly pass the SCP command as argument
                        process.StartInfo.UseShellExecute = false;
                        process.StartInfo.RedirectStandardOutput = true;
                        process.StartInfo.RedirectStandardError = true;

                        process.Start();
                        string output = await process.StandardOutput.ReadToEndAsync();
                        string error = await process.StandardError.ReadToEndAsync();
                        await process.WaitForExitAsync();

                        if (!string.IsNullOrWhiteSpace(error))
                            await Application.Current.MainPage.DisplayAlert("SCP Error", error, "OK");

                        await RunBlurScript();
                    }
                }
                catch (Exception ex)
                {
                    await Application.Current.MainPage.DisplayAlert("Error", $"Failed to execute SCP command: {ex.Message}", "OK");
                }
                finally
                {
                    DeviceIpAddress = string.Empty;
                }
            }
            else
            {
                await Application.Current.MainPage.DisplayAlert("Error", "Invalid IP address entered.", "OK");
            }
        }

        [RelayCommand]
        private async Task RunBlurScript()
        {
            try
            {
                Console.WriteLine("IsDataFetched value " + IsDataFetched);
                // Command to 'cd' into the directory and then run the script with python3
                string command = $"cd '/mnt/c/Users/alexi/Desktop/Assignments/Year 4/Final Year Project/Code/Final-Year-Project/Desktop Application/DesktopApp/' && python3 blur_videos.py";

                using (Process process = new Process())
                {
                    process.StartInfo.FileName = "wsl.exe";
                    process.StartInfo.Arguments = $"-e bash -c \"{command}\"";
                    process.StartInfo.UseShellExecute = false;
                    process.StartInfo.RedirectStandardOutput = true;
                    process.StartInfo.RedirectStandardError = true;

                    process.Start();
                    string output = await process.StandardOutput.ReadToEndAsync();
                    string error = await process.StandardError.ReadToEndAsync();
                    await process.WaitForExitAsync();

                    // If there's an error during script execution, show it
                    if (!string.IsNullOrWhiteSpace(error))
                    {
                        await Application.Current.MainPage.DisplayAlert("Blurring Error", error, "OK");
                        return;
                    }

                    // Show success message after script completes
                    await Application.Current.MainPage.DisplayAlert("Success", "Videos downloaded successfully!", "OK");

                    FetchVideoFiles();

                    Console.WriteLine("IsDataFetched value " + IsDataFetched);
                }
            }
            catch (Exception ex)
            {
                await Application.Current.MainPage.DisplayAlert("Error", $"Failed to execute blur_videos.py: {ex.Message}", "OK");
            }
        }

        [RelayCommand]
        private async Task FetchVideoFiles()
        {
            try
            {
                // Set IsDataFetched to false to hide the UI until data is fetched
                IsDataFetched = false;
                OnPropertyChanged(nameof(IsDataFetched));

                string videoFolderPath;

                // Check user role and select the appropriate folder
                if (IsAdmin)
                {
                    videoFolderPath = Path.Combine(AppDomain.CurrentDomain.BaseDirectory, @"..\..\..\..\", "data");
                }
                else
                {
                    videoFolderPath = Path.Combine(AppDomain.CurrentDomain.BaseDirectory, @"..\..\..\..\", "processed_videos");
                }

                // Asynchronously fetch the video files from the selected folder, including subdirectories
                var videoFiles = await Task.Run(() =>
                {
                    // Determine the video extension based on the folder
                    string searchPattern = IsAdmin ? "*.mp4" : "*.avi";

                    // Use Directory.GetFiles to find all video files in subdirectories
                    return Directory.GetFiles(videoFolderPath, searchPattern, SearchOption.AllDirectories)
                                    .ToList();
                });

                // Update the VideoFiles property on the main thread
                VideoFiles = videoFiles;
                Console.WriteLine($"Video Files: {string.Join(", ", VideoFiles)}");

                // Update the UI to show the video list
                IsDataFetched = true;
                OnPropertyChanged(nameof(IsDataFetched));
                OnPropertyChanged(nameof(IsIpEntryVisible));

            }
            catch (Exception ex)
            {
                Console.WriteLine($"Error fetching video files: {ex.Message}");
                await Application.Current.MainPage.DisplayAlert("Error", $"Failed to fetch video files: {ex.Message}", "OK");
            }
        }

        [RelayCommand]
        private async Task SignOut()
        {
            try
            {
                if(Username == "Guest")
                {
                    Console.WriteLine("Successfully signed out as Guest.");
                    await Shell.Current.GoToAsync("//SignIn");
                    return;
                }

                // Attempt to sign out the user
                _authClient.SignOut();

                IsDataFetched = false;
                OnPropertyChanged(nameof(IsDataFetched));
                OnPropertyChanged(nameof(IsIpEntryVisible));

                // Check if the user is actually signed out
                if (_authClient.User == null)  // If no user is logged in, navigate to SignIn page
                {
                    Console.WriteLine("User successfully signed out.");

                    await Shell.Current.GoToAsync("//SignIn"); // Navigate to SignIn page
                }
                else
                {
                    Console.WriteLine("Sign-out failed. User is still logged in.");
                }
            }
            catch (Exception ex)
            {
                // Handle any sign-out errors
                Console.WriteLine($"Error during logout: {ex.Message}");
            }
        }

    }
}
