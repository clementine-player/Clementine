/*
*  LICENSE
*  
*  Copyright (c) 2006, David Nicolson
*  All rights reserved.
*  
*  Redistribution and use in source and binary forms, with or without
*  modification, are permitted provided that the following conditions
*  are met:
*  
*    1. Redistributions of source code must retain the above copyright
*       notice, this list of conditions and the following disclaimer.
*    2. Redistributions in binary form must reproduce the above copyright
*       notice, this list of conditions and the following disclaimer in the
*       documentation and/or other materials provided with the distribution.
*    3. Neither the name of the author nor the names of its contributors
*       may be used to endorse or promote products derived from this software
*       without specific prior written permission.
*  
*  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
*  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
*  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
*  A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT UNLESS REQUIRED BY
*  LAW OR AGREED TO IN WRITING WILL ANY COPYRIGHT HOLDER OR CONTRIBUTOR
*  BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,
*  OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
*  OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, PROFITS; OR
*  BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
*  WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
*  OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
*  EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef __MBID_MP3_H
#define __MBID_MP3_H

#define MBID_BUFFER_SIZE 37

#include <stdio.h>

// -----------------------------------------------------------------------------

void mfile(size_t length, char ret[], FILE *fp, int *s) {
   size_t bytes = fread(ret,1,length,fp);

   if (bytes != length) {
      *s = 0;
   }
}

// -----------------------------------------------------------------------------

int to_synch_safe(char bytes[]) {
   return ((int)bytes[0] << 21) + ((int)bytes[1] << 14) + ((int)bytes[2] << 7) + (int)bytes[3];
}

int to_integer(char (&bytes)[4]) {
   size_t size = 0;
   uint i;
   for (i=0; i < sizeof(bytes); i++) {
      size = size * 256 + ((int)bytes[i] & 0x000000FF);
   }
   return static_cast<int>(size);
}

// -----------------------------------------------------------------------------

int getMP3_MBID(const char *path, char mbid[MBID_BUFFER_SIZE]) 
{
   FILE *fp;
   static int s = 1;
   char head[3];
   char version[2];
   char flag[1];
   char size[4];
   char size_extended[4];
   int tag_size = 0;
   int extended_size = 0;
   char frame[4];
   char frame_header[4];
   int frame_size;
   int version_major;//, version_minor;

   if (path == NULL) {
      //debug("Received null path\n");
      return -1;
   }

   fp = fopen(path,"rb");
   if (fp == NULL) {
      //debug("Failed to open music file: %s\n",path);
      return -1;
   }

   while (s) {
      mfile(3,head,fp,&s);
      if (!strncmp(head,"ID3",3) == 0) {
         //debug("No ID3v2 tag found: %s\n",path);
         break;
      }

      mfile(2,version,fp,&s);
      version_major = (int)version[0];
      //version_minor = (int)version[1];
      if (version_major == 2) {
         //debug("ID3v2.2.0 does not support MBIDs: %s\n",path);
         break;
      }
      if (version_major != 3 && version_major != 4) {
         //debug("Unsupported ID3 version: v2.%d.%d\n",version_major,version_minor);
         break;
      }

      mfile(1,flag,fp,&s);
      if ((unsigned int)flag[0] & 0x00000040) {
         //debug("Extended header found\n");
         if (version[0] == 4) {
            mfile(4,size_extended,fp,&s);
            extended_size = to_synch_safe(size_extended);
         } else {
            mfile(4,size_extended,fp,&s);
            extended_size = to_integer(size_extended);
         }
         //debug("Extended header size: %d\n",extended_size);
         fseek(fp,extended_size,SEEK_CUR);
      }

      mfile(4,size,fp,&s);
      tag_size = to_synch_safe(size);
      //debug("Tag size: %d\n",tag_size);

      while (s) {
         if (ftell(fp) > tag_size || ftell(fp) > 1048576) {
            break;
         }

         mfile(4,frame,fp,&s);
         if (frame[0] == 0x00) {
            break;
         }
         if (version_major == 4) {
            mfile(4,frame_header,fp,&s);
            frame_size = to_synch_safe(frame_header);
         } else {
            mfile(4,frame_header,fp,&s);
            frame_size = to_integer(frame_header);
         }

         fseek(fp,2,SEEK_CUR);
         //debug("Reading %d bytes from frame %s\n",frame_size,frame);

         if (strncmp(frame,"UFID",4) == 0) {
            //char frame_data[frame_size];
            char frame_data[59];
            mfile(59,frame_data,fp,&s);
            if (frame_size >= 59 && strncmp(frame_data,"http://musicbrainz.org",22) == 0) {
               char *tmbid = frame_data;
               tmbid = frame_data + 23;
               strncpy(mbid,tmbid,MBID_BUFFER_SIZE-1);
               mbid[MBID_BUFFER_SIZE-1] = 0x00;
               fclose(fp);
               return 0;
            }
         } else {
            fseek(fp,frame_size,SEEK_CUR);
         }
      }
      break;
   }

   if (fp) {
      fclose(fp);
   }
   //if (!s) {
   //   debug("Failed to read music file: %s\n",path);
   //}                
   return -1;

}

#endif

// -----------------------------------------------------------------------------
