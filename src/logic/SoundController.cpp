//
// Created by desktop on 19.06.17.
//

#include "SoundController.h"
#include <ZenLib/zenload/zTypes.h>
#include <engine/BaseEngine.h>
#include <engine/World.h>

using namespace Logic;

SoundController::SoundController(World::WorldInstance& world, Handle::EntityHandle entity)
    : Controller(world, entity)
{
}

void SoundController::importObject(const json& j)
{
    Controller::importObject(j);
}

void SoundController::exportPart(json& j)
{
    Controller::exportPart(j);
}

void SoundController::playSound(const std::string& sound)
{
    m_PlayedSound = m_World.getEngine()->getAudioWorld().playSound(sound, getEntityTransform().Translation(), m_SoundMaxDistance);

    m_NumTimesPlayed++;
}

void SoundController::initFromVobDescriptor(const ZenLoad::zCVobData& vob)
{
    m_SoundFile = vob.zCVobSound.sndName;
    m_SoundPlayDelay = vob.zCVobSound.sndRandDelay;
    m_SoundDelayRandomness = vob.zCVobSound.sndRandDelayVar;
    m_SoundMode = vob.zCVobSound.sndType;
    m_SoundMaxDistance = vob.zCVobSound.sndRadius / 100.0f;
}

void SoundController::onUpdate(float deltaTime)
{
    switch (m_SoundMode)
    {
        case ZenLoad::SM_LOOPING:
            if ((!m_PlayedSound || m_PlayedSound->state() != Audio::State::Playing) && isInHearingRange())
            {
                playSound(m_SoundFile);
            }
            break;

        case ZenLoad::SM_ONCE:
            /*if(m_NumTimesPlayed == 0) // FIXME: Should probably be activated by a trigger
            {
                playSound(m_SoundFile);
            }*/
            break;

        case ZenLoad::SM_RANDOM:
            // Loop this sound after some time
            float totalSeconds = (float)m_World.getEngine()->getGameClock().getTotalSecondsRealtime();

            // If that is the first time playing, just calculate the random amount without playing the sound.
            // This will keep sound vobs from all starting at the same time
            if (m_NumTimesPlayed == 0)
            {
                setNextPlayingTimeRandomized();
            }
            else if (m_PlayedSound && totalSeconds >= m_SoundTimePlayNextRandom && m_PlayedSound->state() != Audio::State::Playing && isInHearingRange())
            {
                playSound(m_SoundFile);

                // Calculate on what time this should be played again
                setNextPlayingTimeRandomized();
            }
            break;
    }
}

void SoundController::setNextPlayingTimeRandomized()
{
    // See when we need to play this sound the next time
    float totalSeconds = (float)m_World.getEngine()->getGameClock().getTotalSecondsRealtime();
    float offset = m_SoundPlayDelay + bx::lerp(-m_SoundDelayRandomness, m_SoundDelayRandomness, Utils::frand());

    m_SoundTimePlayNextRandom = totalSeconds + offset;
}

bool SoundController::isInHearingRange()
{
    Math::float3 cam = m_World.getCameraComp<Components::PositionComponent>().m_WorldMatrix.Translation();

    return (getEntityTransform().Translation() - cam).lengthSquared() < m_SoundMaxDistance * m_SoundMaxDistance;
}
