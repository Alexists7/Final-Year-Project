using CommunityToolkit.Mvvm.ComponentModel;
using CommunityToolkit.Mvvm.Input;
using Firebase.Auth;

namespace DesktopApp.Pages
{
    public partial class SignUpViewModel : ObservableObject
    {
        private readonly FirebaseAuthClient _authClient;

        [ObservableProperty]
        private string _email;

        [ObservableProperty]
        private string _username;

        [ObservableProperty]
        private string _password;

        public SignUpViewModel(FirebaseAuthClient authClient)
        {
            _authClient = authClient;
        }

        [RelayCommand]
        private async Task SignUp()
        {
            try
            {
                if (string.IsNullOrWhiteSpace(Email) || string.IsNullOrWhiteSpace(Password) || string.IsNullOrWhiteSpace(Username))
                {
                    await Application.Current.MainPage.DisplayAlert("Error", "Email, Username and Password cannot be empty.", "OK");
                    return;
                }

                // Attempt to create a new user with email, password, and username
                await _authClient.CreateUserWithEmailAndPasswordAsync(Email, Password, Username);

                // Navigate to the SignIn page after successful sign-up
                await Shell.Current.GoToAsync("//SignIn");

                // Optionally display a success message
                await Application.Current.MainPage.DisplayAlert("Success", "Account created successfully. Please sign in.", "OK");
            }
            catch (FirebaseAuthException ex)
            {
                await Application.Current.MainPage.DisplayAlert("Error", $"Sign-up error: {ex.Message}", "OK");
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
                Username = string.Empty;
            }
        }


        [RelayCommand]
        private async Task NavigateSignIn()
        {
            await Shell.Current.GoToAsync("//SignIn");
        }
    }
}
