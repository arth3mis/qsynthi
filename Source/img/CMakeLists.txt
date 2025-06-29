cmake_minimum_required(VERSION 3.22)

add_compile_options(-fPIC)

juce_add_binary_data(BinaryData SOURCES
        attack.png
        decay.png
        filter.png
        filterEnv.png
        filterFreq.png
        filterKey.png
        filterRes.png
        gian.png
        logo.png
        LogoBackground.png
        LogoTransparent.png
        portamento.png
        potentialHeight.png
        potentialScale.png
        potentialShape.png
        potentialShift.png
        release.png
        reverb.png
        simAccuracy.png
        simOffset.png
        simSpeed.png
        stereo.png
        sustain.png
        voiceCount.png
        waveScale.png
        waveShape.png
        waveShift.png
        buttonDonate.png
        buttonHelp.png
        buttonVersion.png

        Inter-Light.ttf
        Inter-Medium.ttf
)