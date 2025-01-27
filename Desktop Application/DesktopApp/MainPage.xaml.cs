namespace DesktopApp
{
    public partial class MainPage : ContentPage
    {
        public MainPage()
        {
            InitializeComponent();
        }

        private async void OnLoginClicked(object sender, EventArgs e)
        {
            // Navigate to the "Sign In" page using the route defined in AppShell.xaml
            await Shell.Current.GoToAsync("///SignIn");
        }
    }
}
