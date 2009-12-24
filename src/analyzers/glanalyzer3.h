/***************************************************************************
                        glanalyzer3.h  -  description 
                             ------------------- 
    begin                : Feb 16 2004 
    copyright            : (C) 2004 by Enrico Ros 
    email                : eros.kde@email.it 
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <config.h>
#ifdef HAVE_QGLWIDGET

#ifndef GLBOUNCER_H
#define GLBOUNCER_H

#include "analyzerbase.h"
#include <qstring.h>
#include <qptrlist.h>

class QWidget;
class Ball;
class Paddle;

class GLAnalyzer3 : public Analyzer::Base3D
{
public:
    GLAnalyzer3(QWidget *);
    ~GLAnalyzer3();
    void analyze( const Scope & );
    void paused();

protected:
    void initializeGL();
    void resizeGL( int w, int h );
    void paintGL();

private:
    struct ShowProperties {
	double timeStamp;
	double dT;
	float colorK;
	float gridScrollK;
	float gridEnergyK;
	float camRot;
	float camRoll;
	float peakEnergy;
    } show;

    struct FrameProperties {
	bool silence;
	float energy;
	float dEnergy;
    } frame;
    
    static const int NUMBER_OF_BALLS = 16;
    
    QPtrList<Ball> balls;
    Paddle * leftPaddle, * rightPaddle;
    float unitX, unitY;
    GLuint ballTexture;
    GLuint gridTexture;

    void drawDot3s( float x, float y, float z, float size );
    void drawHFace( float y );
    void drawScrollGrid( float scroll, float color[4] );

    bool loadTexture(QString file, GLuint& textureID);
    void freeTexture(GLuint& textureID);
};

#endif
#endif
