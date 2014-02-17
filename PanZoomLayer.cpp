
#include "PanZoomLayer.h"

PanZoomLayer::PanZoomLayer()
{

}


PanZoomLayer::~PanZoomLayer()
{

}


PanZoomLayer* PanZoomLayer::create()
{
	PanZoomLayer *pRet = new PanZoomLayer;
	if( pRet && pRet->init() )
	{
		pRet->autorelease();
		return pRet;
	}

	CC_SAFE_DELETE( pRet );
	return NULL;
}


bool PanZoomLayer::init()
{
	if( CCLayerColor::initWithColor( ccc4( 0, 0, 255, 0 ) ) == false )
		return false;

	this->setAnchorPoint( ccp( 0, 0 ) );

	_touches = CCArray::create();
	_touches->retain();

	_accelerationFactor = 0.0f;
	_productFactor = 55.0f;

	_maxScale = 2.5f;
	_minScale = 1.0f;

	_isHolding = false;

	return true;
}


void PanZoomLayer::onEnter()
{
	CCLayer::onEnter();
	CCDirector::sharedDirector()->getScheduler()->scheduleUpdateForTarget(this, 0, false);

	this->setTouchEnabled( true );
}


void PanZoomLayer::onExit()
{
	CCDirector::sharedDirector()->getScheduler()->unscheduleAllForTarget( this );
	CCLayer::onExit();

	_touches->removeAllObjects();
	CC_SAFE_RELEASE_NULL( _touches );
}

void PanZoomLayer::update( float dt )
{
	// Skip smoothe panning when dt is high value
	if( dt > 1.0f / 55 )
		return;

	CCLayerColor::update( dt );

	if( _touches->count() == 1 )
	{
		_accelerationFactor *= 40 * dt * 0.95f;
	}
	else if( _touches->count() == 0 )
	{
		_accelerationFactor = fabs( _accelerationFactor - 0 );
		if( _accelerationFactor < FLT_EPSILON )
			return;

		if( _accelerationFactor < 0.004f )
		{
			_accelerationFactor = 0;
		}
		else
		{
			double d = dt * 60;
			if( d > 0.99 )
				d = 0.99;
			double i = (0 - _accelerationFactor) * 0.025 * d;

			_accelerationFactor = ( _accelerationFactor + i ) * d;

			CCPoint adder = _deltaSum;
			adder.x *= this->getContentSize().width;
			adder.y *= this->getContentSize().height;

			this->setPosition( this->getPosition() + adder * 2.5 * _accelerationFactor );
		}
	}
}


void PanZoomLayer::ccTouchesBegan(CCSet *pTouches, CCEvent *pEvent)
{
	if( _isHolding ) return;

	CCTouch *pTouch;
	CCSetIterator setIter;
	int cnt = 0;
	for (setIter = pTouches->begin(); setIter != pTouches->end(); ++setIter)
	{
		pTouch = (CCTouch *)(*setIter);
		_touches->addObject(pTouch);
	}

	_deltaSum = ccp( 0, 0 );
	_accelerationFactor = 0;
	CCTime::gettimeofdayCocos2d( &_timeStamp, NULL );
}


void PanZoomLayer::ccTouchesMoved(CCSet *pTouches, CCEvent *pEvent)
{
	if( _isHolding ) return;

	if( _touches->count() == 1 )
	{
		CCTouch *touch = (CCTouch*)_touches->objectAtIndex( 0 );
		CCPoint curTouchPosition = CCDirector::sharedDirector()->convertToGL( touch->getLocationInView() );
		CCPoint prevTouchPosition = CCDirector::sharedDirector()->convertToGL( touch->getPreviousLocationInView() );
		CCPoint deltaPosition = curTouchPosition - prevTouchPosition;
		this->setPosition( this->getPosition() + deltaPosition );

		float prevAngle = CC_RADIANS_TO_DEGREES( _prevDeltaPoint.getAngle() );
		float angle = CC_RADIANS_TO_DEGREES( deltaPosition.getAngle() );
		if( fabs( prevAngle - angle ) <= 30 )
		{
			_deltaSum = ccp( 0, 0 );
		}

		_prevDeltaPoint = deltaPosition;
		
		_deltaSum.x = _deltaSum.x + deltaPosition.x / this->getContentSize().width;
		_deltaSum.y = _deltaSum.y + deltaPosition.y / this->getContentSize().height;

		_accelerationFactor += _deltaSum.getLength() * 4.0;
	}
	else if( _touches->count() >= 2 )
	{
		// Get the two first touches
		CCTouch *touch1 = (CCTouch*)_touches->objectAtIndex(0);
		CCTouch *touch2 = (CCTouch*)_touches->objectAtIndex(1);

		// Get current and previous positions of the touches
		CCPoint curPosTouch1 = CCDirector::sharedDirector()->convertToGL(touch1->getLocationInView());
		CCPoint curPosTouch2 = CCDirector::sharedDirector()->convertToGL(touch2->getLocationInView());
		CCPoint prevPosTouch1 = CCDirector::sharedDirector()->convertToGL(touch1->getPreviousLocationInView());
		CCPoint prevPosTouch2 = CCDirector::sharedDirector()->convertToGL(touch2->getPreviousLocationInView());

		// Calculate current and previous positions of the layer relative the anchor point
		CCPoint curPosLayer = ccpMidpoint(curPosTouch1, curPosTouch2);
		CCPoint prevPosLayer = ccpMidpoint(prevPosTouch1, prevPosTouch2);

		// Calculate new scale
		float prevScale = this->getScale();
		float curScale = this->getScale() * ccpDistance(curPosTouch1, curPosTouch2) / ccpDistance(prevPosTouch1, prevPosTouch2);

		this->setScale( curScale );

		if( this->getScale() != prevScale )
		{
			CCPoint realCurPosLayer = this->convertToNodeSpaceAR(curPosLayer);
			float deltaX = (realCurPosLayer.x) * (this->getScale() - prevScale);
			float deltaY = (realCurPosLayer.y) * (this->getScale() - prevScale);

			this->setPosition(ccp(this->getPosition().x - deltaX, this->getPosition().y - deltaY));
		}

		// If current and previous position of the multitouch's center aren't equal -> change position of the layer
		if (!prevPosLayer.equals(curPosLayer))
		{
			this->setPosition(ccp(this->getPosition().x + curPosLayer.x - prevPosLayer.x,
				this->getPosition().y + curPosLayer.y - prevPosLayer.y));
		}
	}
}


void PanZoomLayer::ccTouchesEnded(CCSet *pTouches, CCEvent *pEvent)
{
	if( _isHolding ) return;

	CCTouch *pTouch;
	CCSetIterator setIter;
	for (setIter = pTouches->begin(); setIter != pTouches->end(); ++setIter)
	{
		pTouch = (CCTouch *)(*setIter);
		_touches->removeObject(pTouch);
	}
}


void PanZoomLayer::setPosition( CCPoint position )
{
	CCNode::setPosition( position );

	if( _panBoundsRect.equals( CCRectZero ) == false )
	{
		CCRect boundBox;
		boundBox.origin = this->getPosition() / this->getScale();
		boundBox.size = this->getContentSize() / this->getScale();
		//CCLog( "boundBox : origin(%.1f, %.1f), size(%.1f, %.1f)", boundBox.origin.x, boundBox.origin.y, boundBox.size.width, boundBox.size.height );

		// OpenGL coordinate system
		float left = boundBox.getMinX();
		float right = boundBox.getMaxX();
		float top = boundBox.getMaxY();
		float bottom = boundBox.getMinY();
		//CCLog( "left,right(%.1f, %.1f), top,bottom(%.1f, %.1f)", left, right, top, bottom );

		float min_x = _panBoundsRect.getMinX() + boundBox.size.width;
		float max_x = _panBoundsRect.getMaxX() + boundBox.size.width;
		float min_y = _panBoundsRect.getMinY() + boundBox.size.height;
		float max_y = _panBoundsRect.getMaxY() + boundBox.size.height;
		//CCLog( "min(%.1f, %.1f), max(%.1f, %.1f)", min_x, min_y, max_x, max_y );

		float scale = this->getScale();
		float arLeft = min_x * scale;
		float arRight = max_x * scale - this->getContentSize().width;
		float arTop = max_y * scale - this->getContentSize().height;
		float arBottom = min_y * scale;
		
		if( left < min_x )
		{
			CCNode::setPosition( arLeft, this->getPosition().y );
		}

		if( right > max_x )
		{
			CCNode::setPosition( arRight, this->getPosition().y );
		}

		if( top > max_y )
		{
			CCNode::setPosition( this->getPosition().x, arTop );
		}

		if( bottom < min_y )
		{
			CCNode::setPosition( this->getPosition().x, arBottom );
		}
	}
}


void PanZoomLayer::setScale( float scale )
{
	CCLayerColor::setScale( MIN( MAX( scale, _minScale ), _maxScale ) );
	this->setPosition( this->getPosition() );
}


void PanZoomLayer::SetPanBoundsRect( CCRect rect )
{
	_panBoundsRect = rect;

	CCSize visibleSize = CCDirector::sharedDirector()->getVisibleSize();
	float wFactor = _panBoundsRect.size.width / visibleSize.width;
	float hFactor = _panBoundsRect.size.height / visibleSize.height;
	float minScale;
	if( wFactor > hFactor )
		minScale = wFactor;
	else
		minScale = hFactor;
	SetMinScale( minScale );
}


void PanZoomLayer::SetMaxScale( float maxScale )
{
	_maxScale = maxScale;
}


float PanZoomLayer::GetMaxScale()
{
	return _maxScale;
}


void PanZoomLayer::SetMinScale( float minScale )
{
	_minScale = minScale;
}


float PanZoomLayer::GetMinScale()
{
	return _minScale;
}


void PanZoomLayer::Holding()
{
	_isHolding = true;
}


void PanZoomLayer::UnHolding()
{
	_isHolding = false;
}


void PanZoomLayer::SetProductFactor( float v )
{
	_productFactor = v;
}