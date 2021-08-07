
/* Boutons */
const int buttonCount = 10; //Nombre de boutons
int buttonStates[buttonCount];   //Etat des boutons

void setup()
{
    initializeButtonStates();
}

void loop() {}

//On initialise tous les états de boutons à "LOW" (non enfoncés)
void initializeButtonStates()
{
    for (int i = 0; i < buttonCount; i++)
    {
        buttonStates[i] = LOW;
    }
}