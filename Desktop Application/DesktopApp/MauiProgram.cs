using DesktopApp.Pages;
using Firebase.Auth;
using Firebase.Auth.Providers;
using Firebase.Auth.Repository;
using Microsoft.Extensions.Logging;

namespace DesktopApp
{
    public static class MauiProgram
    {
        public static MauiApp CreateMauiApp()
        {
            var builder = MauiApp.CreateBuilder();
            builder
                .UseMauiApp<App>()
                .ConfigureFonts(fonts =>
                {
                    fonts.AddFont("OpenSans-Regular.ttf", "OpenSansRegular");
                    fonts.AddFont("OpenSans-Semibold.ttf", "OpenSansSemibold");
                });

#if DEBUG
    		builder.Logging.AddDebug();
#endif

			builder.Services.AddSingleton(new FirebaseAuthClient(new FirebaseAuthConfig()
			{
				ApiKey = "AIzaSyDNo90y6YlLD-Spj791nCtwWquqtAR0h6I",
				AuthDomain = "desktopapp-8d7b1.firebaseapp.com",
				Providers = new FirebaseAuthProvider[]
				{
					new EmailProvider()
				},
				//UserRepository = new FileUserRepository("DesktopApp")
			}));

            builder.Services.AddSingleton<SignInView>();
			builder.Services.AddSingleton<SignInViewModel>();
            builder.Services.AddSingleton<SignUpView>();
            builder.Services.AddSingleton<SignUpViewModel>();
            builder.Services.AddSingleton<DisplayPage>();
            builder.Services.AddSingleton<DisplayPageViewModel>();

            return builder.Build();


        }
    }

}
