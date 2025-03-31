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

            PlayVideoCommand = new RelayCommand(PlayVideo, CanPlayVideo);
        }

        private bool CanPlayVideo()
        {
            return IsVideoSelected;
        }

        private void PlayVideo()
        {
            if (!string.IsNullOrWhiteSpace(SelectedVideoPath))
            {
                Console.WriteLine($"Playing video from: {SelectedVideoPath}");
            }
            else
            {
                Console.WriteLine("No video selected.");
            }
        }

        public void OnVideoSelected(string videoPath)
        {
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

                    string command = $"sshpass -p \"{scpPassword}\" scp -r \"root@{DeviceIpAddress}:/home/ashaju/Platform Test/3) Face Detection/data\" ./DesktopApp";

                    using (Process process = new Process())
                    {
                        process.StartInfo.FileName = "wsl.exe";
                        process.StartInfo.Arguments = command;
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

                    if (!string.IsNullOrWhiteSpace(error))
                    {
                        await Application.Current.MainPage.DisplayAlert("Blurring Error", error, "OK");
                        return;
                    }

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
                IsDataFetched = false;
                OnPropertyChanged(nameof(IsDataFetched));

                string videoFolderPath;

                if (IsAdmin)
                {
                    videoFolderPath = Path.Combine(AppDomain.CurrentDomain.BaseDirectory, @"..\..\..\..\", "data");
                }
                else
                {
                    videoFolderPath = Path.Combine(AppDomain.CurrentDomain.BaseDirectory, @"..\..\..\..\", "processed_videos");
                }

                var videoFiles = await Task.Run(() =>
                {
                    string searchPattern = IsAdmin ? "*.mp4" : "*.avi";

                    return Directory.GetFiles(videoFolderPath, searchPattern, SearchOption.AllDirectories)
                                    .ToList();
                });

                VideoFiles = videoFiles;
                Console.WriteLine($"Video Files: {string.Join(", ", VideoFiles)}");

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

                _authClient.SignOut();

                IsDataFetched = false;
                OnPropertyChanged(nameof(IsDataFetched));
                OnPropertyChanged(nameof(IsIpEntryVisible));

                if (_authClient.User == null)
                {
                    Console.WriteLine("User successfully signed out.");

                    await Shell.Current.GoToAsync("//SignIn");
                }
                else
                {
                    Console.WriteLine("Sign-out failed. User is still logged in.");
                }
            }
            catch (Exception ex)
            {
                Console.WriteLine($"Error during logout: {ex.Message}");
            }
        }

    }
}
