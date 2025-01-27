using CommunityToolkit.Mvvm.ComponentModel;
using CommunityToolkit.Mvvm.Input;
using Firebase.Auth;

namespace DesktopApp.Pages
{
    public partial class DisplayPageViewModel : ObservableObject
    {
        private readonly FirebaseAuthClient _authClient;

        public string Username => _authClient.User?.Info?.DisplayName ?? "Guest";

        public DisplayPageViewModel(FirebaseAuthClient authClient)
        {
            _authClient = authClient;
        }

        public void OnAppearing()
        {
            OnPropertyChanged(nameof(Username));
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
