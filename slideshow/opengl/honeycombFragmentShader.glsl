/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#version 150

in vec2 texturePosition;
in float fuzz;
in vec2 v_center;
in vec3 normal;

uniform sampler2D slideTexture;
uniform float selectedTexture;
uniform float time;
uniform float hexagonSize;

bool isBorder(vec2 point)
{
    return point.x < 0.02 || point.x > 0.98 || point.y < 0.02 || point.y > 0.98;
}

void main()
{
    vec4 fragment = texture2D(slideTexture, texturePosition);
    vec3 lightVector = vec3(0.0, 0.0, 1.0);
    float light = max(dot(lightVector, normal), 0.0);
    if (hexagonSize > 1.0) {
        // The space in-between hexagons.
        if (selectedTexture > 0.5)
            fragment.a = 1.0 - time * 8 + gl_FragCoord.x / 1024.;
        else
            fragment.a = time * 8 - 7.3 + gl_FragCoord.x / 1024.;
    } else {
        // The hexagons themselves.

        float startTime;
        float actualTime;
        if (selectedTexture > 0.5) {
            // Leaving slide.
            if (isBorder(v_center))
                // If the center is “outside” of the canvas, clear it first.
                startTime = 0.15;
            else
                startTime = 0.15 + fuzz * 0.4;
            float endTime = startTime + 0.05;
            actualTime = 1.0 - clamp((time - startTime) / (endTime - startTime), 0, 1);
        } else {
            // Entering slide.
            if (isBorder(v_center))
                // If the center is “outside” of the canvas, clear it first.
                startTime = 0.85;
            else
                startTime = 0.3 + fuzz * 0.4;
            float endTime = startTime + 0.05;
            actualTime = clamp((time - startTime) / (endTime - startTime), 0, 1);
            if (time < 0.8)
                actualTime *= time / 0.8;
        }

        if (selectedTexture > 0.5) {
            // Leaving texture needs to be transparent to see-through.
            fragment.a = actualTime;
        } else {
            // Entering one though, would look weird with transparency.
            fragment.rgb *= actualTime;
        }
    }
    vec4 black = vec4(0.0, 0.0, 0.0, fragment.a);
    gl_FragColor = mix(black, fragment, light);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
