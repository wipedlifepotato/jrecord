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
static unsigned int rate = 48000;
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
    par.rate = rate/2;
    par.rchan = 2;
    par.pchan = 0;
    par.bits = 16;
    par.bps = 2;
    par.sig = 1;
    par.le = 1;
    par.msb = 1;
    if (!sio_setpar(handler, &par)) {
      throw std::runtime_error("Could not set audio parameters");
    }
    if (!sio_getpar(handler,&par)) {
        throw std::runtime_error("sio_getpar failed");
    }
    if (par.bits != 16 || par.bps != 2 || par.sig != 1 || par.le != 1) {
	    throw std::runtime_error("device does not support 16-bit little-endian PCM");
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
  byte m_buf[2048];

  std::shared_ptr<buffer> get_data(size_t count = 1024) {
    auto b = std::make_shared<buffer>();
    b->size = count;
    b->data = std::shared_ptr<byte[]>(new byte[count]);

    size_t total = 0;
    while (total < count) {
        auto v = sio_read(m_handler_sio, b->data.get() + total, count - total);
        if (v == 0) break;
        total += v;
    }
    b->size = total;
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
   std::string cmd = "ffmpeg -f x11grab -i :0.0 "
                  "-f s16le -ar "+std::to_string(rate)+" -ac 1 -i pipe:0 "
                  "-af \"aresample=async=1\" "
                  "-c:v libx264 -preset ultrafast -c:a aac -b:a 128k " + out;
    FILE *f =
        popen(
              cmd.c_str(),
              "w");
    if (!f) {
      throw std::runtime_error("Can't call ffmpeg for x11grab");
    }
    mPopen = f;
    write_sound_thread = std::thread([this]() {
      //	std::cout << "RUNS" << std::endl;
      while (running) {
    	std::lock_guard<std::mutex> lock(mtx);
        //		std::cout << "write sound" << std::endl;
        write_sound();
      }
    });
  }
  void stop(void) {
    std::lock_guard<std::mutex> lock(mtx);
    running = false;
    if (mPopen)
      pclose(mPopen);
  }
  void write_sound() {
    auto n = m_recorder.get_data();

    std::lock_guard<std::mutex> lock(mtx);
    if (n->size > 0 && mPopen) {
        fwrite(n->data.get(), 1, n->size, mPopen);
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
#ifdef DEBUG_ONLY_VOICE
  	auto rec = audio_recorder();
 	auto data = rec.get_data();
   for (auto i = 0; i< data->size;i++){
//  	std::cout << char(data->data[i]) << std::endl;
  }
   FILE* audio_file = fopen("/tmp/audio.pcm", "wb");
   for (int i = 1000;i>0;i--) {
	data = rec.get_data();
   	fwrite(data->data.get(), 1, data->size, audio_file);
   }
   exit(0);
#endif
  if (signal(SIGINT, close_program) == SIG_ERR) {
    fprintf(stderr, "An error occurred while setting a signal handler.\n");
    return EXIT_FAILURE;
  }
  int opt;
  std::string audio_dev{};
  std::string out_path{};
  while ((opt = getopt(argc, argv, "f:a:hr:")) != -1) {
        switch (opt) {
            case 'f':
                out_path = optarg;
                break;
            case 'a':
                audio_dev = optarg;
                break;
            case 'r':
				rate = atoi(optarg);
				break;
            default:
                fprintf(stderr, "Usage: %s [-f output_file] [-a audio_device] [-r if your voice is bad try to use -r 94100. for a while idk why so. need to think]\n", argv[0]);
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
