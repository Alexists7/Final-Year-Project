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

        public string Username => _authClient.User?.Info?.DisplayName ?? "Guest";
        public string UserRole => IsAdmin ? "Admin" : "User";

        public DisplayPageViewModel(FirebaseAuthClient authClient)
        {
            _authClient = authClient;
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

                        // Display SCP output and error
                        if (!string.IsNullOrWhiteSpace(output))
                            await Application.Current.MainPage.DisplayAlert("SCP Output", output, "OK");

                        if (!string.IsNullOrWhiteSpace(error))
                            await Application.Current.MainPage.DisplayAlert("SCP Error", error, "OK");

                        await Application.Current.MainPage.DisplayAlert("Sucess", "Downloaded motion recordings!", "OK");
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
