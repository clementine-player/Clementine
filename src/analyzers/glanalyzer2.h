/***************************************************************************
                        glanalyzer2.h  -  description
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

#ifndef GLSTARVIEW_H
#define GLSTARVIEW_H

#include <config.h>
#ifdef HAVE_QGLWIDGET

#include "analyzerbase.h"
#include <qstring.h>
#include <qptrlist.h>


class GLAnalyzer2 : public Analyzer::Base3D
{
public:
    GLAnalyzer2(QWidget *);
    ~GLAnalyzer2();
    void analyze( const Scope & );
    void paused();

protected:
    void initializeGL();
    void resizeGL( int w, int h );
    void paintGL();

private:
    struct ShowProperties {
    bool paused;
    double timeStamp;
    double dT;
    double pauseTimer;
    float rotDegrees;
    } show;

    struct FrameProperties {
    float energy;
    float dEnergy;
    float meanBand;
    float rotDegrees;
    bool silence;
    } frame;

    GLuint dotTexture;
    GLuint w1Texture;
    GLuint w2Texture;
    float unitX, unitY;

    void drawDot( float x, float y, float size );
    void drawFullDot( float r, float g, float b, float a );
    void setTextureMatrix( float rot, float scale );

    bool loadTexture(QString file, GLuint& textureID);
    void freeTexture(GLuint& textureID);
};

#endif
#endif
