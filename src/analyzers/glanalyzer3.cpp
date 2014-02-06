/***************************************************************************
                      glanalyzer3.cpp  -  Bouncing Ballzz
                             -------------------
    begin                : Feb 19 2004
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

#include <cmath>
#include <cstdlib>
#include "glanalyzer3.h"
#include <kdebug.h>
#include <kstandarddirs.h>
#include <qimage.h>
#include <sys/time.h>

#ifndef HAVE_FABSF
inline float fabsf(float f)
{
    return f < 0.f ? -f : f;
}
#endif


class Ball
{
    public:
    Ball() : x( drand48() - drand48() ), y( 1 - 2.0 * drand48() ),
        z( drand48() ), vx( 0.0 ), vy( 0.0 ), vz( 0.0 ),
        mass( 0.01 + drand48()/10.0 )
        //,color( (float[3]) { 0.0, drand48()*0.5, 0.7 + drand48() * 0.3 } )
    {
        //this is because GCC < 3.3 can't compile the above line, we aren't sure why though
        color[0] = 0.0; color[1] = drand48()*0.5; color[2] = 0.7 + drand48() * 0.3;
    };

    float x, y, z, vx, vy, vz, mass;
    float color[3];

    void updatePhysics( float dT )
    {
        x += vx * dT;                // position
        y += vy * dT;                // position
        z += vz * dT;                // position
        if ( y < -0.8 ) vy = fabsf( vy );
        if ( y > 0.8 ) vy = -fabsf( vy );
        if ( z < 0.1 ) vz = fabsf( vz );
        if ( z > 0.9 ) vz = -fabsf( vz );
        vx += (( x > 0 ) ? 4.94 : -4.94) * dT;    // G-force
        vx *= (1 - 2.9 * dT);            // air friction
        vy *= (1 - 2.9 * dT);            // air friction
        vz *= (1 - 2.9 * dT);            // air friction
    }
};

class Paddle
{
    public:
    Paddle( float xPos ) : onLeft( xPos < 0 ), mass( 1.0 ),
        X( xPos ), x( xPos ), vx( 0.0 ) {};

    void updatePhysics( float dT )
    {
        x += vx * dT;                // posision
        vx += (1300 * (X - x) / mass) * dT;        // elasticity
        vx *= (1 - 4.0 * dT);            // air friction
    }

    void renderGL()
    {
        glBegin( GL_TRIANGLE_STRIP );
         glColor3f( 0.0f, 0.1f, 0.3f );
         glVertex3f( x, -1.0f, 0.0 );
         glVertex3f( x, 1.0f, 0.0 );
         glColor3f( 0.1f, 0.2f, 0.6f );
         glVertex3f( x, -1.0f, 1.0 );
         glVertex3f( x, 1.0f, 1.0 );
        glEnd();
    }

    void bounce( Ball * ball )
    {
        if ( onLeft && ball->x < x )
        {
        ball->vx = vx * mass / (mass + ball->mass) + fabsf( ball->vx );
        ball->vy = (drand48() - drand48()) * 1.8;
        ball->vz = (drand48() - drand48()) * 0.9;
        ball->x = x;
        }
        else if ( !onLeft && ball->x > x )
        {
        ball->vx = vx * mass / (mass + ball->mass) - fabsf( ball->vx );
        ball->vy = (drand48() - drand48()) * 1.8;
        ball->vz = (drand48() - drand48()) * 0.9;
        ball->x = x;
        }
    }

    void impulse( float strength )
    {
        if ( (onLeft && strength > vx) || (!onLeft && strength < vx) )
        vx += strength;
    }

    private:
    bool onLeft;
    float mass, X, x, vx;
};


GLAnalyzer3::GLAnalyzer3( QWidget *parent ):
Analyzer::Base3D(parent, 15)
{
    //initialize openGL context before managing GL calls
    makeCurrent();
    loadTexture( locate("data","amarok/data/ball.png"), ballTexture );
    loadTexture( locate("data","amarok/data/grid.png"), gridTexture );

    balls.setAutoDelete( true );
    leftPaddle = new Paddle( -1.0 );
    rightPaddle = new Paddle( 1.0 );
    for ( int i = 0; i < NUMBER_OF_BALLS; i++ )
    balls.append( new Ball() );

    show.colorK = 0.0;
    show.gridScrollK = 0.0;
    show.gridEnergyK = 0.0;
    show.camRot = 0.0;
    show.camRoll = 0.0;
    show.peakEnergy = 1.0;
    frame.silence = true;
    frame.energy = 0.0;
    frame.dEnergy = 0.0;
}

GLAnalyzer3::~GLAnalyzer3()
{
    freeTexture( ballTexture );
    freeTexture( gridTexture );
    delete leftPaddle;
    delete rightPaddle;
    balls.clear();
}

void GLAnalyzer3::initializeGL()
{
    // Set a smooth shade model
    glShadeModel(GL_SMOOTH);

    // Disable depth test (all is drawn 'z-sorted')
    glDisable( GL_DEPTH_TEST );

    // Set blending function (Alpha addition)
    glBlendFunc( GL_SRC_ALPHA, GL_ONE );

    // Clear frame with a black background
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
}

void GLAnalyzer3::resizeGL( int w, int h )
{
    // Setup screen. We're going to manually do the perspective projection
    glViewport( 0, 0, (GLint)w, (GLint)h );
    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();
    glFrustum( -0.5f, 0.5f, -0.5f, 0.5f, 0.5f, 4.5f );

    // Get the aspect ratio of the screen to draw 'circular' particles
    float ratio = (float)w / (float)h;
    if ( ratio >= 1.0 ) {
    unitX = 0.34 / ratio;
    unitY = 0.34;
    } else {
    unitX = 0.34;
    unitY = 0.34 * ratio;
    }

    // Get current timestamp.
    timeval tv;
    gettimeofday( &tv, nullptr );
    show.timeStamp = (double)tv.tv_sec + (double)tv.tv_usec/1000000.0;
}

void GLAnalyzer3::paused()
{
    analyze( Scope() );
}

void GLAnalyzer3::analyze( const Scope &s )
{
    // compute the dTime since the last call
    timeval tv;
    gettimeofday( &tv, nullptr );
    double currentTime = (double)tv.tv_sec + (double)tv.tv_usec/1000000.0;
    show.dT = currentTime - show.timeStamp;
    show.timeStamp = currentTime;

    // compute energy integrating frame's spectrum
    if ( !s.empty() )
    {
    int bands = s.size();
    float currentEnergy = 0,
          maxValue = 0;
    // integrate spectrum -> energy
    for ( int i = 0; i < bands; i++ )
    {
        float value = s[i];
        currentEnergy += value;
        if ( value > maxValue )
        maxValue = value;
    }
    currentEnergy *= 100.0 / (float)bands;
    // emulate a peak detector: currentEnergy -> peakEnergy (3tau = 30 seconds)
    show.peakEnergy = 1.0 + ( show.peakEnergy - 1.0 ) * exp( - show.dT / 10.0 );
    if ( currentEnergy > show.peakEnergy )
        show.peakEnergy = currentEnergy;
    // check for silence
    frame.silence = currentEnergy < 0.001;
    // normalize frame energy against peak energy and compute frame stats
    currentEnergy /= show.peakEnergy;
    frame.dEnergy = currentEnergy - frame.energy;
    frame.energy = currentEnergy;
    } else
    frame.silence = true;

    // update the frame
    updateGL();
}

void GLAnalyzer3::paintGL()
{
    // limit max dT to 0.05 and update color and scroll constants
    if ( show.dT > 0.05 )
    show.dT = 0.05;
    show.colorK += show.dT * 0.4;
    if ( show.colorK > 3.0 )
    show.colorK -= 3.0;
    show.gridScrollK += 0.2 * show.peakEnergy * show.dT;

    // Switch to MODEL matrix and clear screen
    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();
    glClear( GL_COLOR_BUFFER_BIT );

    // Draw scrolling grid
    if ( (show.gridEnergyK > 0.05) || (!frame.silence && frame.dEnergy < -0.3) )
    {
    show.gridEnergyK *= exp( -show.dT / 0.1 );
    if ( -frame.dEnergy > show.gridEnergyK )
        show.gridEnergyK = -frame.dEnergy*2.0;
    float gridColor[4] = { 0.0, 1.0, 0.6, show.gridEnergyK };
    drawScrollGrid( show.gridScrollK, gridColor );
    }

    // Roll camera up/down handling the beat
    show.camRot += show.camRoll * show.dT;        // posision
    show.camRoll -= 400 * show.camRot * show.dT;    // elasticity
    show.camRoll *= (1 - 2.0 * show.dT);        // friction
    if ( !frame.silence && frame.dEnergy > 0.4 )
    show.camRoll += show.peakEnergy*2.0;
    glRotatef( show.camRoll / 2.0, 1,0,0 );

    // Translate the drawing plane
    glTranslatef( 0.0f, 0.0f, -1.8f );

    // Draw upper/lower planes and paddles
    drawHFace( -1.0 );
    drawHFace( 1.0 );
    leftPaddle->renderGL();
    rightPaddle->renderGL();

    // Draw Balls
    if ( ballTexture ) {
    glEnable( GL_TEXTURE_2D );
    glBindTexture( GL_TEXTURE_2D, ballTexture );
    } else
    glDisable( GL_TEXTURE_2D );
    glEnable( GL_BLEND );
    Ball * ball = balls.first();
    for ( ; ball; ball = balls.next() )
    {
    float color[3],
          angle = show.colorK;
    // Rotate the color based on 'angle' value [0,3)
    if ( angle < 1.0 )
    {
        color[ 0 ] = ball->color[ 0 ] * (1 - angle) + ball->color[ 1 ] * angle;
        color[ 1 ] = ball->color[ 1 ] * (1 - angle) + ball->color[ 2 ] * angle;
        color[ 2 ] = ball->color[ 2 ] * (1 - angle) + ball->color[ 0 ] * angle;
    }
    else if ( angle < 2.0 )
    {
        angle -= 1.0;
        color[ 0 ] = ball->color[ 1 ] * (1 - angle) + ball->color[ 2 ] * angle;
        color[ 1 ] = ball->color[ 2 ] * (1 - angle) + ball->color[ 0 ] * angle;
        color[ 2 ] = ball->color[ 0 ] * (1 - angle) + ball->color[ 1 ] * angle;
    }
    else
    {
        angle -= 2.0;
        color[ 0 ] = ball->color[ 2 ] * (1 - angle) + ball->color[ 0 ] * angle;
        color[ 1 ] = ball->color[ 0 ] * (1 - angle) + ball->color[ 1 ] * angle;
        color[ 2 ] = ball->color[ 1 ] * (1 - angle) + ball->color[ 2 ] * angle;
    }
    // Draw the dot and update its physics also checking at bounces
    glColor3fv( color );
    drawDot3s( ball->x, ball->y, ball->z, 1.0 );
    ball->updatePhysics( show.dT );
    if ( ball->x < 0 )
        leftPaddle->bounce( ball );
    else
        rightPaddle->bounce( ball );
    }
    glDisable( GL_BLEND );
    glDisable( GL_TEXTURE_2D );

    // Update physics of paddles
    leftPaddle->updatePhysics( show.dT );
    rightPaddle->updatePhysics( show.dT );
    if ( !frame.silence )
    {
    leftPaddle->impulse( frame.energy*3.0 + frame.dEnergy*6.0 );
    rightPaddle->impulse( -frame.energy*3.0 - frame.dEnergy*6.0 );
    }
}

void GLAnalyzer3::drawDot3s( float x, float y, float z, float size )
{
    // Circular XY dot drawing functions
    float sizeX = size * unitX,
      sizeY = size * unitY,
      pXm = x - sizeX,
      pXM = x + sizeX,
      pYm = y - sizeY,
      pYM = y + sizeY;
    // Draw the Dot
    glBegin( GL_QUADS );
    glTexCoord2f( 0, 0 );    // Bottom Left
    glVertex3f( pXm, pYm, z );
    glTexCoord2f( 0, 1 );    // Top Left
    glVertex3f( pXm, pYM, z );
    glTexCoord2f( 1, 1 );    // Top Right
    glVertex3f( pXM, pYM, z );
    glTexCoord2f( 1, 0 );    // Bottom Right
    glVertex3f( pXM, pYm, z );
    glEnd();

    // Shadow XZ drawing functions
    float sizeZ = size / 10.0,
      pZm = z - sizeZ,
      pZM = z + sizeZ,
      currentColor[4];
    glGetFloatv( GL_CURRENT_COLOR, currentColor );
    float alpha = currentColor[3],
      topSide = (y + 1) / 4,
      bottomSide = (1 - y) / 4;
    // Draw the top shadow
    currentColor[3] = topSide * topSide * alpha;
    glColor4fv( currentColor );
    glBegin( GL_QUADS );
    glTexCoord2f( 0, 0 );    // Bottom Left
    glVertex3f( pXm, 1, pZm );
    glTexCoord2f( 0, 1 );    // Top Left
    glVertex3f( pXm, 1, pZM );
    glTexCoord2f( 1, 1 );    // Top Right
    glVertex3f( pXM, 1, pZM );
    glTexCoord2f( 1, 0 );    // Bottom Right
    glVertex3f( pXM, 1, pZm );
    glEnd();
    // Draw the bottom shadow
    currentColor[3] = bottomSide * bottomSide * alpha;
    glColor4fv( currentColor );
    glBegin( GL_QUADS );
    glTexCoord2f( 0, 0 );    // Bottom Left
    glVertex3f( pXm, -1, pZm );
    glTexCoord2f( 0, 1 );    // Top Left
    glVertex3f( pXm, -1, pZM );
    glTexCoord2f( 1, 1 );    // Top Right
    glVertex3f( pXM, -1, pZM );
    glTexCoord2f( 1, 0 );    // Bottom Right
    glVertex3f( pXM, -1, pZm );
    glEnd();
}

void GLAnalyzer3::drawHFace( float y )
{
    glBegin( GL_TRIANGLE_STRIP );
    glColor3f( 0.0f, 0.1f, 0.2f );
    glVertex3f( -1.0f, y, 0.0 );
    glVertex3f( 1.0f, y, 0.0 );
    glColor3f( 0.1f, 0.6f, 0.5f );
    glVertex3f( -1.0f, y, 2.0 );
    glVertex3f( 1.0f, y, 2.0 );
    glEnd();
}

void GLAnalyzer3::drawScrollGrid( float scroll, float color[4] )
{
    if ( !gridTexture )
    return;
    glMatrixMode( GL_TEXTURE );
    glLoadIdentity();
    glTranslatef( 0.0, -scroll, 0.0 );
    glMatrixMode( GL_MODELVIEW );
    float backColor[4] = { 1.0, 1.0, 1.0, 0.0 };
    for ( int i = 0; i < 3; i++ )
    backColor[ i ] = color[ i ];
    glEnable( GL_TEXTURE_2D );
    glBindTexture( GL_TEXTURE_2D, gridTexture );
    glEnable( GL_BLEND );
    glBegin( GL_TRIANGLE_STRIP );
     glColor4fv( color );    // top face
    glTexCoord2f( 0.0f, 1.0f );
    glVertex3f( -1.0f, 1.0f, -1.0f );
    glTexCoord2f( 1.0f, 1.0f );
    glVertex3f( 1.0f, 1.0f, -1.0f );
     glColor4fv( backColor );    // central points
    glTexCoord2f( 0.0f, 0.0f );
    glVertex3f( -1.0f, 0.0f, -3.0f );
    glTexCoord2f( 1.0f, 0.0f );
    glVertex3f( 1.0f, 0.0f, -3.0f );
     glColor4fv( color );    // bottom face
    glTexCoord2f( 0.0f, 1.0f );
    glVertex3f( -1.0f, -1.0f, -1.0f );
    glTexCoord2f( 1.0f, 1.0f );
    glVertex3f( 1.0f, -1.0f, -1.0f );
    glEnd();
    glDisable( GL_BLEND );
    glDisable( GL_TEXTURE_2D );
    glMatrixMode( GL_TEXTURE );
    glLoadIdentity();
    glMatrixMode( GL_MODELVIEW );
}

bool GLAnalyzer3::loadTexture( QString fileName, GLuint& textureID )
{
    //reset texture ID to the default EMPTY value
    textureID = 0;

    //load image
    QImage tmp;
    if ( !tmp.load( fileName ) )
    return false;

    //convert it to suitable format (flipped RGBA)
    QImage texture = QGLWidget::convertToGLFormat( tmp );
    if ( texture.isNull() )
    return false;

    //get texture number and bind loaded image to that texture
    glGenTextures( 1, &textureID );
    glBindTexture( GL_TEXTURE_2D, textureID );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexImage2D( GL_TEXTURE_2D, 0, 4, texture.width(), texture.height(),
    0, GL_RGBA, GL_UNSIGNED_BYTE, texture.bits() );
    return true;
}

void GLAnalyzer3::freeTexture( GLuint& textureID )
{
    if ( textureID > 0 )
    glDeleteTextures( 1, &textureID );
    textureID = 0;
}

#endif
