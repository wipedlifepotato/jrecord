#include <iostream>
#include <mutex>
#include <regex>
#include <signal.h>
#include <sndio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <thread>
#include <unistd.h>
///
//////
typedef unsigned char byte;
class audio_recorder {
  struct buffer {
    size_t size;
    std::shared_ptr<unsigned char[]> data;
  };

private:
  struct sio_hdl *open_record(std::string r) {
    auto handler = sio_open(r.c_str(), SIO_REC, 0);
    std::cout << "open_record " << r << std::endl;
    if (!handler) {
      throw std::runtime_error("Can't open audio recorder " + r);
    }
    std::cout << " set_audio recorder " << std::endl;
    struct sio_par par;
    sio_initpar(&par);
    par.bits = 16;
    par.sig = 1;
    par.le = 1;
    par.rate = 44100;
    par.pchan = 1;
    par.appbufsz = 44100 / 25;
    if (!sio_setpar(handler, &par)) {
      throw std::runtime_error("Could not set audio parameters");
    }
    if (!sio_start(handler)) {
      throw std::runtime_error("sio_start err");
    }
    return handler;
  }

protected:
  struct sio_hdl *m_handler_sio;

public:
  audio_recorder(std::string recorder) : m_handler_sio(open_record(recorder)) {}
  audio_recorder()
      : m_handler_sio(open_record(getenv("AUDIODEVICE") ? getenv("AUDIODEVICE")
                                                        : "snd/0")) {}
  ~audio_recorder() {
    if (m_handler_sio)
      sio_stop(m_handler_sio);
    if (m_handler_sio)
      sio_close(m_handler_sio);
  }
  std::shared_ptr<buffer> get_data(size_t count = 512) {
    auto b = std::make_shared<buffer>();
    b->size = count;
    b->data = std::shared_ptr<byte[]>(new byte[count]);
    auto c = 0;
    while (c < count) {
      auto v = sio_read(m_handler_sio, b->data.get() + c, count);
      if (v == 0)
        break;
      c += v;
    }
    return b;
  }
};
#include <cstdio>
class VideoRecorder {
private:
  audio_recorder m_recorder;
  FILE *mPopen;
  std::mutex mtx;
  std::thread write_sound_thread;
  std::atomic<bool> running{true};

public:
  bool is_running(void) { return running; }
  void join(void) { write_sound_thread.join(); }
  // void terminate(void) = nullptr;
  VideoRecorder(std::string out = "/tmp/out.mp4",
                const char *audio_dev = nullptr)
      : m_recorder(audio_dev ? audio_recorder(audio_dev) : audio_recorder()),
        mPopen(nullptr) {
    bool is_file = false;
    // TODO: fix regex
    //((\w+)+\/?)+|(rtmp.*?@(.*?):(\d+)\/(\w+)\/(\w+))
    std::regex path_regex(R"([^\0]+)");
    if (std::regex_match(out, path_regex)) {
      is_file = true;
      // throw std::runtime_error("bad outpath for video");
    }
    if (is_file && access(out.c_str(), F_OK) == 0) {
      char answer;
      while (answer != 'y' && answer != 'n') {
        std::cout << "File is exist need to drop? [y/n] ";
        std::cin >> answer;
      }
      if (answer == 'n') {
        std::cout << "close the program" << std::endl;
        exit(0);
      }
      remove(out.c_str());
    }

    FILE *f =
        popen(("ffmpeg -f x11grab -flush_packets 1 -i :0.0 -f s16le -ar 44100 "
               "-ac 1 -i pipe:0 -c:v libx264 -c:a aac -preset ultrafast " +
               out + " >/dev/null")
                  .c_str(),
              "w");
    if (!f) {
      throw std::runtime_error("Can't call ffmpeg for x11grab");
    }
    mPopen = f;
    write_sound_thread = std::thread([this]() {
      //	std::cout << "RUNS" << std::endl;
      while (running) {
        //		std::cout << "write sound" << std::endl;
        write_sound(20);
      }
    });
  }
  void stop(void) {
    running = false;
    if (mPopen)
      pclose(mPopen);
  }
  void write_sound(size_t chunks_ms = 20) { // seconds) {
    /*
                    auto bufferDataSize2Seconds = [&seconds]() {
                            return 44100 * seconds;
                    };
    */
    size_t bytes_per_chunk = (44100 * 2 * chunks_ms) / 1000;

    if (mPopen) {
      std::lock_guard<std::mutex> lock(mtx);
      auto data = m_recorder.get_data(bytes_per_chunk);
      if (!fwrite(data->data.get(), 1, data->size, mPopen)) {
        std::cout << data->data.get()[0] << std::endl;
        throw std::runtime_error("can't write sound");
      }
      fflush(mPopen);

#ifdef DEBUG
      long long sum = 0;
      for (size_t i = 0; i < data->size; i++)
        sum += (signed char)data->data[i];

      if (sum == 0) {
        std::cout << "SILENCE FROM SNDIO!" << std::endl;
      }
      static FILE *f = fopen("raw_dump.pcm", "wb");
      if (f) {
        fwrite(data->data.get(), 1, data->size, f);
        fflush(f);
      }
#endif
    }
  }
  ~VideoRecorder() { stop(); }
};
VideoRecorder *global_recorder = nullptr;
void close_program(int signo) {
  if (global_recorder) {
    global_recorder->stop();
  }
}
int main(int argc, char **argv, char **env) {
  //	auto rec = audio_recorder();
  //	auto data = rec.get_data();
  // for (auto i = 0; i< data->size;i++){
  //	std::cout << char(data->data[i]) << std::endl;
  //}
  
  if (signal(SIGINT, close_program) == SIG_ERR) {
    fprintf(stderr, "An error occurred while setting a signal handler.\n");
    return EXIT_FAILURE;
  }
  int opt;
  std::string audio_dev{};
  std::string out_path{};
  while ((opt = getopt(argc, argv, "f:a:h")) != -1) {
        switch (opt) {
            case 'f':
                out_path = optarg;
                break;
            case 'a':
                audio_dev = optarg;
                break;
            default:
                fprintf(stderr, "Usage: %s [-f output_file] [-a audio_device]\n", argv[0]);
                return EXIT_FAILURE;
        }
  }

  auto r = VideoRecorder(out_path.size() ? out_path : "/tmp/out.mp4", audio_dev.size() ? audio_dev.c_str() : nullptr);
  global_recorder = &r;
  while (r.is_running()) {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }
  return 0;
}
