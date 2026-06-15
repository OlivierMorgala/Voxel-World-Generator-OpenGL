#version 330 core
out vec4 FragColor;

in vec3 ourColor;  // Kolor odebrany z vertex shadera
in vec2 texCoord;  // Współrzędne ściany (0.0 - 1.0)
in vec3 FragPosition;

uniform float alpha;
uniform bool isBorderRendered;
uniform bool isUnderwater;


/// RAYCAST
uniform vec3 blockHitPosition;
uniform float BlockHasHit; // 1 - trafiono blok 2 - nie trafiono bloku

void main()
{
    // Parametry obramowania
    float thickness = 0.05; // Grubość ramki
    
    // Sprawdzamy, czy aktualny piksel znajduje się przy którejkolwiek krawędzi
    bool isBorder = (texCoord.x < thickness || texCoord.x > 1.0 - thickness || 
                     texCoord.y < thickness || texCoord.y > 1.0 - thickness);

// Obliczamy, czy ten piksel należy do bloku, który aktualnie trafiamy raycastem
   float errorMargin = 0.001;
        bool equalBlocksPosition = (
            FragPosition.x >= blockHitPosition.x - errorMargin && FragPosition.x <= blockHitPosition.x + 1.0 + errorMargin &&
            FragPosition.y >= blockHitPosition.y - errorMargin && FragPosition.y <= blockHitPosition.y + 1.0 + errorMargin &&
            FragPosition.z >= blockHitPosition.z - errorMargin && FragPosition.z <= blockHitPosition.z + 1.0 + errorMargin
        );
     
    vec4 finalColor;

    if (isBorder && isBorderRendered) {

        // Rysujemy czarną ramkę albo jezeli raycast trafi to podswietlamy ramke bloku
        if(BlockHasHit == 1.0 && equalBlocksPosition)
        {
          finalColor = vec4(ourColor + 0.3f, alpha);
         }
         else
         {
         finalColor = vec4(0.0, 0.0, 0.0, alpha);
         }

    } else {
        // Rysujemy właściwy kolor bloku
        finalColor = vec4(ourColor, alpha);
    }

    if(isUnderwater == true){
        vec3 waterColor = vec3(0.02, 0.15, 0.45);

        float depth = gl_FragCoord.z / gl_FragCoord.w;
        float fogDensity = 0.04;

        float fogFactor = exp(-pow((depth * fogDensity), 2.0));
        fogFactor = clamp(fogFactor, 0.0, 1.0);

        vec3 tintedColor = mix(waterColor, finalColor.rgb, fogFactor);
        tintedColor *= 0.6;
        FragColor = vec4(tintedColor, finalColor.a);
    }else{

        FragColor = finalColor;

    }

}