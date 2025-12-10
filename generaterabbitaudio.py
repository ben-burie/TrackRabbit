from bark import generate_audio
from scipy.io.wavfile import write as write_wav
import os

def text_to_bark_audio(text, output_path, voice_preset="v2/en_speaker_0"):
    print(f"Generating audio with preset: {voice_preset}")

    audio_array = generate_audio(
        text,
        history_prompt=voice_preset
    )

    write_wav(output_path + ".wav", 24000, audio_array)
    print(f"Saved to {output_path}")



if __name__ == "__main__":

    os.makedirs("outputs2", exist_ok=True)

    commands = [
      "You are right on pace!",
      "You are 1 second behind!",
      "You are 2 seconds behind!",
      "You are 3 seconds behind!",
      "You are 4 seconds behind!",
      "You are 5 seconds behind!",
      "You are 6 seconds behind!",
      "You are 7 seconds behind!",
      "You are 8 seconds behind!",
      "You are 9 seconds behind!",
      "You are 10 seconds behind!",
      "You are 1 second ahead!",
      "You are 2 seconds ahead!",
      "You are 3 seconds ahead!",
      "You are 4 seconds ahead!",
      "You are 5 seconds ahead!",
      "You are 6 seconds ahead!",
      "You are 7 seconds ahead!",
      "You are 8 seconds ahead!",
      "You are 9 seconds ahead!",
      "You are 10 seconds ahead!",
      "You are way behind!",
      "You are way ahead!"
    ]

    labels = [
      "onpace",
      "1behind",
      "2behind",
      "3behind",
      "4behind",
      "5behind",
      "6behind",
      "7behind",
      "8behind",
      "9behind",
      "10behind",
      "1ahead",
      "2ahead",
      "3ahead",
      "4ahead",
      "5ahead",
      "6ahead",
      "7ahead",
      "8ahead",
      "9ahead",
      "10ahead",
      "waybehind",
      "wayahead"
    ]

    labelIndex = 0;

    for command in commands:
        output_path = os.path.join("outputs2", labels[labelIndex])
        text_to_bark_audio(command, output_path, voice_preset="v2/en_speaker_0", )
        labelIndex += 1