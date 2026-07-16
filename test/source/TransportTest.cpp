#include <catch2/catch_test_macros.hpp>
#include "harmonic_engine/AudioEngine/Transport.h"

using namespace harmonic_engine;

TEST_CASE("Transport basic operations", "[transport]")
{
    Transport transport;

    SECTION("Initial state")
    {
        REQUIRE_FALSE(transport.isPlaying());
        REQUIRE_FALSE(transport.isRecording());
        REQUIRE_FALSE(transport.isPaused());
        REQUIRE(transport.getPositionInSamples() == 0);
        REQUIRE(transport.getTempo() == 120.0);
    }

    SECTION("Start and stop")
    {
        transport.start();
        REQUIRE(transport.isPlaying());
        REQUIRE_FALSE(transport.isPaused());

        transport.stop();
        REQUIRE_FALSE(transport.isPlaying());
        REQUIRE_FALSE(transport.isPaused());
    }

    SECTION("Pause toggles")
    {
        transport.start();
        REQUIRE(transport.isPlaying());

        transport.pause();
        REQUIRE_FALSE(transport.isPlaying());
        REQUIRE(transport.isPaused());

        transport.pause();
        REQUIRE(transport.isPlaying());
        REQUIRE_FALSE(transport.isPaused());
    }

    SECTION("Record mode")
    {
        transport.record();
        REQUIRE(transport.isPlaying());
        REQUIRE(transport.isRecording());
    }

    SECTION("Position management")
    {
        transport.setPositionInSamples(44100);
        REQUIRE(transport.getPositionInSamples() == 44100);
        REQUIRE(transport.getPositionInSeconds() == 1.0);

        transport.setPositionInSeconds(2.0);
        REQUIRE(transport.getPositionInSamples() == 88200);
    }

    SECTION("Tempo")
    {
        transport.setTempo(140.0);
        REQUIRE(transport.getTempo() == 140.0);
    }
}
