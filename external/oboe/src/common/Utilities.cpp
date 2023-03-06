/*
 * Copyright 2016 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdlib.h>
#include <unistd.h>

#ifdef __ANDROID__
#include <sys/system_properties.h>
#endif

#include <oboe/AudioStream.h>
#include "oboe/Definitions.h"
#include "oboe/Utilities.h"

namespace oboe {

constexpr float kScaleI16ToFloat = (1.0f / 32768.0f);

void convertFloatToPcm16(const float *source, int16_t *destination, int32_t numSamples) {
    for (int i = 0; i < numSamples; i++) {
        float fval = source[i];
        fval += 1.0; // to avoid discontinuity at 0.0 caused by truncation
        fval *= 32768.0f;
        auto sample = static_cast<int32_t>(fval);
        // clip to 16-bit range
        if (sample < 0) sample = 0;
        else if (sample > 0x0FFFF) sample = 0x0FFFF;
        sample -= 32768; // center at zero
        destination[i] = static_cast<int16_t>(sample);
    }
}

void convertPcm16ToFloat(const int16_t *source, float *destination, int32_t numSamples) {
    for (int i = 0; i < numSamples; i++) {
        destination[i] = source[i] * kScaleI16ToFloat;
    }
}

int32_t convertFormatToSizeInBytes(AudioFormat format) {
    int32_t size = 0;
    switch (format) {
        case AudioFormat::I16:
            size = sizeof(int16_t);
            break;
        case AudioFormat::Float:
            size = sizeof(float);
            break;
        case AudioFormat::I24:
            size = 3; // packed 24-bit data
            break;
        case AudioFormat::I32:
            size = sizeof(int32_t);
            break;
        default:
            break;
    }
    return size;
}

void getPropertyString(const char * name, char* out) {
    out[0] = 0;
#ifdef __ANDROID__
    if (__system_property_get(name, out) != 0) {

    }
    else {
        out[0] = 0;
    }
#else
    (void) name;
#endif
}

int getPropertyInteger(const char * name, int defaultValue) {
    int result = defaultValue;
#ifdef __ANDROID__
    char valueText[PROP_VALUE_MAX] = {0};
    if (__system_property_get(name, valueText) != 0) {
        result = atoi(valueText);
    }
#else
    (void) name;
#endif
    return result;
}

int getSdkVersion() {
    static int sCachedSdkVersion = -1;
#ifdef __ANDROID__
    if (sCachedSdkVersion == -1) {
        sCachedSdkVersion = getPropertyInteger("ro.build.version.sdk", -1);
    }
#endif
    return sCachedSdkVersion;
}

}// namespace oboe
