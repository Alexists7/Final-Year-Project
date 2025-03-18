namespace DesktopApp.Pages;

public partial class SignInView : ContentPage
{
	public SignInView(SignInViewModel viewModel)
	{
		InitializeComponent();

		BindingContext = viewModel;
	}
}
