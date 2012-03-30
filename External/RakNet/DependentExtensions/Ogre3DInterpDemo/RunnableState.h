#ifndef __RUNNABLE_STATE_H
#define __RUNNABLE_STATE_H

#include "AppTypes.h"
#include "State.h"

class AppInterface;

class RunnableState : public State
{
public:
	virtual void Update(AppTime curTimeMS, AppTime elapsedTimeMS)=0;
	virtual void UpdateEnd(AppTime curTimeMS, AppTime elapsedTimeMS) {};
	virtual void Render(AppTime curTimeMS)=0;
	void SetParentApp(AppInterface *parent);
	// Call when internal variables set and the state is ready to use
	virtual void OnStateReady(void);

	// Don't do this so I can override by return type in derived classes
	//virtual AppInterface *GetParentApp(void) const;

	virtual void SetFocus(bool hasFocus);

protected:
	AppInterface *parent;

};


#endif
