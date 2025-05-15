
using namespace std;



int main()
{
    int choice;
    cout << "Enter 1 for server and 0 for Client: \n";
    cin >> choice;
    int themeChoice;
    cout << "enter your theme choice\n1 for fire and Ice\n2 for forest and wood\n3 for neon\n4 for futuristic\n5 for underwater\n6 for spaceGalaxy\n";
    cin >> themeChoice;
    THEMES currentTheme;
    switch (themeChoice)
    {
    case 1:
        currentTheme = THEME1;
        break;
    case 2:
        currentTheme = THEME2;
        break;
    case 3:
        currentTheme = THEME3;
        break;
    case 4:
        currentTheme = THEME4;
        break;
    case 5:
        currentTheme = THEME5;
        break;
    case 6:
        currentTheme = THEME6;
        break;
    default:
        currentTheme = THEME3;
        break;
    }
    if (choice)
    {
        runServer(currentTheme);
    }
    else
    {
        string ipInput;
        cout << "\n Enter IP : ";
        cin >> ipInput;
        runClient(currentTheme, ipInput);
    }
    return 0;
}