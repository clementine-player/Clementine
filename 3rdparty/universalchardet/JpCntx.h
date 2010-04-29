/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is Mozilla Communicator client code.
 *
 * The Initial Developer of the Original Code is
 * Netscape Communications Corporation.
 * Portions created by the Initial Developer are Copyright (C) 1998
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the MPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the MPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */

#ifndef __JPCNTX_H__
#define __JPCNTX_H__

#include <stdint.h>

#define NUM_OF_CATEGORY 6

 

#define ENOUGH_REL_THRESHOLD  100
#define MAX_REL_THRESHOLD     1000

//hiragana frequency category table
extern const uint8_t jp2CharContext[83][83];

class JapaneseContextAnalysis
{
public:
  JapaneseContextAnalysis() {Reset(false);}

  void HandleData(const char* aBuf, uint32_t aLen);

  void HandleOneChar(const char* aStr, uint32_t aCharLen)
  {
    int32_t order;

    //if we received enough data, stop here   
    if (mTotalRel > MAX_REL_THRESHOLD)   mDone = true;
    if (mDone)       return;
     
    //Only 2-bytes characters are of our interest
    order = (aCharLen == 2) ? GetOrder(aStr) : -1;
    if (order != -1 && mLastCharOrder != -1)
    {
      mTotalRel++;
      //count this sequence to its category counter
      mRelSample[jp2CharContext[mLastCharOrder][order]]++;
    }
    mLastCharOrder = order;
  }

  float GetConfidence(void);
  void      Reset(bool aIsPreferredLanguage);
  void      SetOpion(){}
  bool GotEnoughData() {return mTotalRel > ENOUGH_REL_THRESHOLD;}

protected:
  virtual int32_t GetOrder(const char* str, uint32_t *charLen) = 0;
  virtual int32_t GetOrder(const char* str) = 0;

  //category counters, each integer counts sequences in its category
  uint32_t mRelSample[NUM_OF_CATEGORY];

  //total sequence received
  uint32_t mTotalRel;

  //Number of sequences needed to trigger detection
  uint32_t mDataThreshold;
  
  //The order of previous char
  int32_t  mLastCharOrder;

  //if last byte in current buffer is not the last byte of a character, we
  //need to know how many byte to skip in next buffer.
  uint32_t mNeedToSkipCharNum;

  //If this flag is set to true, detection is done and conclusion has been made
  bool   mDone;
};


class SJISContextAnalysis : public JapaneseContextAnalysis
{
  //SJISContextAnalysis(){};
protected:
  int32_t GetOrder(const char* str, uint32_t *charLen);

  int32_t GetOrder(const char* str)
  {
    //We only interested in Hiragana, so first byte is '\202'
    if (*str == '\202' && 
          (unsigned char)*(str+1) >= (unsigned char)0x9f && 
          (unsigned char)*(str+1) <= (unsigned char)0xf1)
      return (unsigned char)*(str+1) - (unsigned char)0x9f;
    return -1;
  }
};

class EUCJPContextAnalysis : public JapaneseContextAnalysis
{
protected:
  int32_t GetOrder(const char* str, uint32_t *charLen);
  int32_t GetOrder(const char* str)
    //We only interested in Hiragana, so first byte is '\244'
  {
    if (*str == '\244' &&
          (unsigned char)*(str+1) >= (unsigned char)0xa1 &&
          (unsigned char)*(str+1) <= (unsigned char)0xf3)
      return (unsigned char)*(str+1) - (unsigned char)0xa1;
    return -1;
  }
};

#endif /* __JPCNTX_H__ */

