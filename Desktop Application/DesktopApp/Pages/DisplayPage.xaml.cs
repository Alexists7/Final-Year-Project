namespace DesktopApp.Pages;

public partial class DisplayPage : ContentPage
{
	public DisplayPage(DisplayPageViewModel viewModel)
	{
		InitializeComponent();

        BindingContext = viewModel;
	}

	protected override void OnAppearing()
	{
		base.OnAppearing();

		// Call the OnAppearing method of the ViewModel
		var viewModel = BindingContext as DisplayPageViewModel;
		viewModel?.OnAppearing();
	}
}
