
// Created By GrowingDever 21th January 2014

#ifndef _PAN_ZOOM_LAYER_H_
#define _PAN_ZOOM_LAYER_H_

#include "cocos2d.h"

using namespace cocos2d;

class PanZoomLayer : public CCLayerColor
{
private:
	CCArray *_touches;
	CCPoint _beganTouchPoint;
	CCPoint _endedTouchPoint;
	CCPoint _deltaSum;
	CCPoint _prevDeltaPoint;
	double _accelerationFactor;
	cc_timeval _timeStamp;
	CCRect _panBoundsRect;
	float _maxScale;
	float _minScale;
	float _productFactor;

	bool _isHolding;


public:
	PanZoomLayer();
	virtual ~PanZoomLayer();

	static PanZoomLayer* create();

	virtual bool init();
	virtual void onEnter();
	virtual void onExit();

	virtual void update( float dt );

	virtual void ccTouchesBegan(CCSet *pTouches, CCEvent *pEvent);
	virtual void ccTouchesMoved(CCSet *pTouches, CCEvent *pEvent);
	virtual void ccTouchesEnded(CCSet *pTouches, CCEvent *pEvent);

	virtual void setPosition( CCPoint position );
	virtual void setScale( float scale );

	void SetPanBoundsRect( CCRect rect );
	void SetMaxScale( float maxScale );
	float GetMaxScale();
	void SetMinScale( float minScale );
	float GetMinScale();

	void Holding();
	void UnHolding();

	void SetProductFactor( float v );

};


#endif