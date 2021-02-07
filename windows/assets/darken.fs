#version 330

// Input fragment attributes (from fragment shader)
in vec2 fragTexCoord;
in vec4 fragColor;

// Input uniform values
uniform float darkness_mod;
uniform sampler2D texture0;

// Output fragment color
out vec4 finalColor;

void main()
{
    // Texel color fetching from texture sampler
    vec4 texelColor = texture(texture0, fragTexCoord)*fragColor;
    finalColor = vec4(texelColor.r / darkness_mod, texelColor.g / darkness_mod, texelColor.b / darkness_mod, texelColor.a);
}