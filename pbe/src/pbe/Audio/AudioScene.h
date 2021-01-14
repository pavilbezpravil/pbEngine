#pragma once

// #include <AL/alc.h>

#include "pbe/Core/Ref.h"
#include "pbe/Core/UUID.h"
#include "pbe/Core/Math/Common.h"

class ALCdevice;
class ALCcontext;

namespace pbe
{
	class Entity;
	class Scene;
	
	namespace audio
	{

		void Init();
		void Term();

		class AudioScene;
		
		class SoundSource
		{
		public:
			// SoundSource(const Ref<AudioScene>& audioScene);
			~SoundSource();
			
			bool Load(const char* filename);
			void Unload();

			void StartPlay();
			void StopPlay();

			bool IsLoaded() const;
			bool IsPlaying() const;
			
			void SetPitch(float v);
			void SetGain(float v);
			void SetPosition(const Vec3& v);
			void SetVelocity(const Vec3& v);
			void SetLopping(bool loop);

			void UpdateAll();
			
			void Default();
			
			float Pitch = 1.0f;
			float Gain = 1.0f;
			bool IsLooping = true;
			bool IsAutoPlay = true;
			
		private:
			unsigned int source; // ALuint
			unsigned int buffer; // ALuint

			bool loaded = false;
			// Ref<AudioScene> pAudioScene;
		};


		class AudioScene : public RefCounted
		{
			friend class AudioMng;
		public:
			AudioScene();
			~AudioScene();

		private:
			ALCcontext* openALContext;
		};


		class AudioMng
		{
		public:
			AudioMng();
			~AudioMng();

			void BindContext(const Ref<AudioScene>& audioContext);
			void UnbindContext();

			ALCdevice* GetOpenALDevice() { return openALDevice; }
			
		private:
			ALCdevice* openALDevice;
		};
		extern AudioMng* s_AudioMng;

	}
}

