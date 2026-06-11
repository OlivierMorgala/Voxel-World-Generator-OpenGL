#version 330 core
out vec4 FragColor;

in vec3 ourColor;  // Kolor odebrany z vertex shadera
in vec2 TexCoord;  // Współrzędne ściany (0.0 - 1.0)

uniform float alpha;
uniform bool isBorderRendered;
uniform bool isUnderwater;

void main()
{
    // Parametry obramowania
    float thickness = 0.05; // Grubość ramki
    
    // Sprawdzamy, czy aktualny piksel znajduje się przy którejkolwiek krawędzi
    bool isBorder = (TexCoord.x < thickness || TexCoord.x > 1.0 - thickness || 
                     TexCoord.y < thickness || TexCoord.y > 1.0 - thickness);

    vec4 finalColor;
    if (isBorder && isBorderRendered) {
        // Rysujemy czarną ramkę
        finalColor = vec4(0.0, 0.0, 0.0, alpha);
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