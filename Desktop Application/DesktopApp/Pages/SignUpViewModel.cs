using CommunityToolkit.Mvvm.ComponentModel;
using CommunityToolkit.Mvvm.Input;
using Firebase.Auth;
using System.Text.RegularExpressions;

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

                if (!IsValidEmail(Email))
                {
                    await Application.Current.MainPage.DisplayAlert("Error", "Please enter a valid email address.", "OK");
                    return;
                }

                if (!IsValidPassword(Password))
                {
                    await Application.Current.MainPage.DisplayAlert("Error", "Password must be at least 8 characters long, contain one uppercase letter, one lowercase letter, and one special character.", "OK");
                    return;
                }

                await _authClient.CreateUserWithEmailAndPasswordAsync(Email, Password, Username);

                await Shell.Current.GoToAsync("//SignIn");

                await Application.Current.MainPage.DisplayAlert("Success", "Account created successfully. Please sign in.", "OK");
            }
            catch (FirebaseAuthException ex)
            {
                await Application.Current.MainPage.DisplayAlert("Error", $"Sign-up error: {ex.Message}", "OK");
            }
            catch (Exception ex)
            {
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

        private bool IsValidEmail(string email)
        {
            var emailRegex = new Regex(@"^[^@\s]+@[^@\s]+\.[^@\s]+$");
            return emailRegex.IsMatch(email);
        }

        private bool IsValidPassword(string password)
        {
            var passwordRegex = new Regex(@"^(?=.*[a-z])(?=.*[A-Z])(?=.*\d)(?=.*[!@#$%^&*()_+={}\[\]:;,.?<>~\\/-]).{8,}$");
            return passwordRegex.IsMatch(password);
        }
    }
}
