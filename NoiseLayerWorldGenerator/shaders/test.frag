#version 330 core
out vec4 FragColor;

in vec3 ourColor;  // Kolor odebrany z vertex shadera
in vec2 TexCoord;  // Współrzędne ściany (0.0 - 1.0)

uniform vec3 blockHitPosition;
uniform float BlockHasHit; // Sprawdzenie czy aktualnie jest trafiany jakiś blok 1 - tak, 0- nie



void main()
{
    // Parametry obramowania
    float thickness = 0.10; // Grubość ramki
    
    // Sprawdzamy, czy aktualny piksel znajduje się przy którejkolwiek krawędzi
    bool isBorder = (TexCoord.x < thickness || TexCoord.x > 1.0 - thickness || 
                     TexCoord.y < thickness || TexCoord.y > 1.0 - thickness);

    if (isBorder) {
        // Rysujemy czarną ramkę albo --- W zależności czy raycast trafił blok

        if(BlockHasHit == 1)
        FragColor = vec4(1.0, 0.0, 0.0, 1.0);
        else
        FragColor = vec4(0.0, 0.0, 0.0, 1.0);
        
        
    } else {
        // Rysujemy właściwy kolor bloku
        FragColor = vec4(ourColor, 1.0);
    }
}