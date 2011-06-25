//
//  echoprint-codegen
//  Copyright 2011 The Echo Nest Corporation. All rights reserved.
//


#include <stdio.h>
#include <string.h>
#include <memory>
#ifndef _WIN32
    #include <libgen.h>
    #include <dirent.h>
#endif
#include <stdlib.h>
#include <stdexcept>

#include "AudioStreamInput.h"
#include "Metadata.h"
#include "Codegen.h"
#include <string>
#define MAX_FILES 200000

#ifdef _WIN32
#include <pthread.h>
#endif

// Struct to pass to the worker threads
typedef struct {
	char *filename;
    int start_offset;
    int duration;
    int done ;
    int tag;
	char *output;
} thread_parm_t;

// Thank you http://stackoverflow.com/questions/150355/programmatically-find-the-number-of-cores-on-a-machine
#ifdef _WIN32
#include <windows.h>
#elif MACOS
#include <sys/param.h>
#include <sys/sysctl.h>
#else
#include <unistd.h>
#endif

int getNumCores() {
#ifdef WIN32
    SYSTEM_INFO sysinfo;
    GetSystemInfo(&sysinfo);
    return sysinfo.dwNumberOfProcessors;
#elif MACOS
    int nm[2];
    size_t len = 4;
    uint32_t count;

    nm[0] = CTL_HW; nm[1] = HW_AVAILCPU;
    sysctl(nm, 2, &count, &len, NULL, 0);

    if(count < 1) {
        nm[1] = HW_NCPU;
        sysctl(nm, 2, &count, &len, NULL, 0);
        if(count < 1) { count = 1; }
    }
    return count;
#else
    return sysconf(_SC_NPROCESSORS_ONLN);
#endif
}

using namespace std;

// deal with quotes etc in json
std::string escape(const string& value) {
    std::string s(value);
    std::string out = "";
    out.reserve(s.size());
    for (size_t i = 0; i < s.size(); i++) {
        char c = s[i];
        if (c <= 31)
            continue;
        
        switch (c) {
            case '"' : out += "\\\""; break;
            case '\\': out += "\\\\"; break;
            case '\b': out += "\\b" ; break;
            case '\f': out += "\\f" ; break;
            case '\n': out += "\\n" ; break;
            case '\r': out += "\\r" ; break;
            case '\t': out += "\\t" ; break;
            // case '/' : out += "\\/" ; break; // Unnecessary?
            default:
                out += c;
                // TODO: do something with unicode?
        }
    }
    
    return out;
}

char* json_string_for_file(char* filename, int start_offset, int duration, int tag) {
    // Given a filename, do all the work to get a JSON string output. 
    // This is called by a thread
    double t1 = now();
    
    auto_ptr<FfmpegStreamInput> pAudio(new FfmpegStreamInput());
    pAudio->ProcessFile(filename, start_offset, duration);
    
    if (pAudio.get() == NULL) { // Unable to decode!
        char* output = (char*) malloc(16384);
        sprintf(output,"{\"error\":\"could not create decoder\", \"tag\":%d, \"metadata\":{\"filename\":\"%s\"}}", 
            tag, 
            escape(filename).c_str());
        return output;
    }
    
    int numSamples = pAudio->getNumSamples();
    
    if (numSamples < 1) {
        char* output = (char*) malloc(16384);
        sprintf(output,"{\"error\":\"could not decode\", \"tag\":%d, \"metadata\":{\"filename\":\"%s\"}}", 
            tag, 
            escape(filename).c_str());
        return output;
    }
    t1 = now() - t1;

    double t2 = now();
    auto_ptr<Codegen> pCodegen(new Codegen(pAudio->getSamples(), numSamples, start_offset));    
    t2 = now() - t2;

    // Get the ID3 tag information.
    auto_ptr<Metadata> pMetadata(new Metadata(filename));

    // preamble + codelen
    char* output = (char*) malloc(sizeof(char)*(16384 + strlen(pCodegen->getCodeString().c_str()) ));
    
    sprintf(output,"{\"metadata\":{\"artist\":\"%s\", \"release\":\"%s\", \"title\":\"%s\", \"genre\":\"%s\", \"bitrate\":%d,"
                    "\"sample_rate\":%d, \"duration\":%d, \"filename\":\"%s\", \"samples_decoded\":%d, \"given_duration\":%d,"
                    " \"start_offset\":%d, \"version\":%2.2f, \"codegen_time\":%2.6f, \"decode_time\":%2.6f}, \"code_count\":%d,"
                    " \"code\":\"%s\", \"tag\":%d}",
        escape(pMetadata->Artist()).c_str(), 
        escape(pMetadata->Album()).c_str(), 
        escape(pMetadata->Title()).c_str(), 
        escape(pMetadata->Genre()).c_str(),
        pMetadata->Bitrate(),
        pMetadata->SampleRate(),
        pMetadata->Seconds(),
        escape(filename).c_str(),
        numSamples,
        duration,
        start_offset,
        pCodegen->getVersion(),
        t2,
        t1,
        pCodegen->getNumCodes(),
        pCodegen->getCodeString().c_str(),
        tag
    );
    return output;
}


void *threaded_json_string_for_file(void *parm) {
    // pthread stub to invoke json_string_for_file
    thread_parm_t *p = (thread_parm_t *)parm;
    char * output = json_string_for_file(p->filename, p->start_offset, p->duration, p->tag);
    p->output = output;
    // mark when we're done so the controlling thread can move on.
    p->done = 1;
    return NULL;
}

void print_json_to_screen(char* output, int count, int done) {
    // Print a json block depending on how many there are and where we are.
    if(done==1 && count>1) printf("[\n%s,\n", output);
        else if(done==1 && count == 1) printf("[\n%s\n]\n", output);
            else if(done == count) printf("%s\n]\n", output);
                else printf("%s,\n", output);
}


int main(int argc, char** argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s [ filename | -s ] [seconds_start] [seconds_duration] [< file_list (if -s is set)]\n", argv[0]);
        exit(-1);
    }
    
    try {
        string files[MAX_FILES];
        char *filename = argv[1];
        int count = 0;
        int start_offset = 0;
        int duration = 0;
        int already = 0;
        if (argc > 2) start_offset = atoi(argv[2]);
        if (argc > 3) duration = atoi(argv[3]);
        if (argc > 4) already = atoi(argv[4]);
        // If you give it -s, it means to read in a list of files from stdin.
        if (strcmp(filename, "-s") == 0) {
            while(cin) {
                if (count < MAX_FILES) {
                    string temp_str;
                    getline(cin, temp_str);
                    if (temp_str.size() > 2)
                        files[count++] = temp_str;
                } else {
                    throw std::runtime_error("Too many files on stdin to process\n");
                }
            }
        } else files[count++] = filename;
        
        if(count == 0) throw std::runtime_error("No files given.\n");


#ifdef _WIN32
//#ifdef __APPLE__
        fprintf(stderr, "no thread mode\n");
        // Threading doesn't work in windows yet.
        for(int i=0;i<count;i++) {
            char* output = json_string_for_file((char*)files[i].c_str(), start_offset, duration, i);
            print_json_to_screen(output, count, i);
            free(output);
        }
        return 1;
#endif

        // FIgure out how many threads to use based on # of cores
        int num_threads = getNumCores();
        if (num_threads > 8) num_threads = 8;
        if (num_threads < 2) num_threads = 2;
        if (num_threads > count) num_threads = count;

        // Setup threading
        pthread_t *t = (pthread_t*)malloc(sizeof(pthread_t)*num_threads);
        thread_parm_t **parm = (thread_parm_t**)malloc(sizeof(thread_parm_t*)*num_threads);
        pthread_attr_t *attr = (pthread_attr_t*)malloc(sizeof(pthread_attr_t)*num_threads);

        // Kick off the first N threads
        int still_left = count-1-already;
        for(int i=0;i<num_threads;i++) {
    	    parm[i] = (thread_parm_t *)malloc(sizeof(thread_parm_t));
            parm[i]->filename = (char*)files[still_left].c_str();
            parm[i]->start_offset = start_offset;
            parm[i]->tag = still_left;
            parm[i]->duration = duration;
            parm[i]->done = 0;
            still_left--;
            pthread_attr_init(&attr[i]);
            pthread_attr_setdetachstate(&attr[i], PTHREAD_CREATE_DETACHED);
            // Kick off the thread
            if (pthread_create(&t[i], &attr[i], threaded_json_string_for_file, (void*)parm[i]))
                throw std::runtime_error("Problem creating thread\n");
        }
        
        int done = 0;
        // Now wait for the threads to come back, and also kick off new ones
        while(done<count) {
            // Check which threads are done
            for(int i=0;i<num_threads;i++) {
                if (parm[i]->done) {
                    parm[i]->done = 0;
                    done++;
                    print_json_to_screen(parm[i]->output, count, done);
                    free(parm[i]->output);
                    // More to do? Start a new one on this just finished thread
                    if(still_left >= 0) {
                        parm[i]->tag = still_left;
                        parm[i]->filename = (char*)files[still_left].c_str();
                        still_left--;
                        int err= pthread_create(&t[i], &attr[i], threaded_json_string_for_file, (void*)parm[i]);
                        if(err) 
                            throw std::runtime_error("Problem creating thread\n");
                        
                    }
                }
            }       
        }

        // Clean up threads
        for(int i=0;i<num_threads;i++) {
            free(parm[i]);
        }
        free(t);
        free(parm);
        free(attr);
        return 0;
    }
    catch(std::runtime_error& ex) {
        fprintf(stderr, "%s\n", ex.what());
        return 1;
    }
    catch(...) {
        fprintf(stderr, "Unknown failure occurred\n");
        return 2;
    }

}
