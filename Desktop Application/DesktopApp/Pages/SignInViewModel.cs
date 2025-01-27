using CommunityToolkit.Mvvm.ComponentModel;
using CommunityToolkit.Mvvm.Input;
using Firebase.Auth;

namespace DesktopApp.Pages
{
    public partial class SignInViewModel : ObservableObject
    {
        private readonly FirebaseAuthClient _authClient;

        [ObservableProperty]
        private string _email;

        [ObservableProperty]
        private string _password;

        public string Username => _authClient.User?.Info?.DisplayName ?? "Guest";

        [ObservableProperty]
        private string _secretMessage;

        public SignInViewModel(FirebaseAuthClient authClient)
        {
            _authClient = authClient;
            Console.WriteLine(Username);
        }

        [RelayCommand]
        private async Task SignIn()
        {
            try
            {
                if (string.IsNullOrWhiteSpace(Email) || string.IsNullOrWhiteSpace(Password))
                {
                    await Application.Current.MainPage.DisplayAlert("Error", "Email and Password cannot be empty.", "OK");
                    return;
                }

                // Attempt to sign in with email and password
                await _authClient.SignInWithEmailAndPasswordAsync(Email, Password);

                // Navigate to the DisplayPage after successful login
                await Shell.Current.GoToAsync("//DisplayPage");
            }
            catch (FirebaseAuthException ex)
            {
                await Application.Current.MainPage.DisplayAlert("Error", $"Authentication error: {ex.Message}", "OK");
            }
            catch (Exception ex)
            {
                // Handle general errors
                await Application.Current.MainPage.DisplayAlert("Error", $"An unexpected error occurred: {ex.Message}", "OK");
            }
            finally
            {
                Email = string.Empty;
                Password = string.Empty;
            }
        }


        [RelayCommand]
        private async Task NavigateSignUp()
        {
            await Shell.Current.GoToAsync("//SignUp");
        }

        [RelayCommand]
        private async Task SignInGuest()
        {
            await Application.Current.MainPage.DisplayAlert("Guest Sign In", "You are now signed in as a guest.", "OK");
            await Shell.Current.GoToAsync("//DisplayPage");
        }
    }
}
