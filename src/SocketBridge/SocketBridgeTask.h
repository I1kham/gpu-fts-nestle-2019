#ifndef _SocketBridgeTask_h_
#define _SocketBridgeTask_h_
#include "../rheaCommonLib/rhea.h"
#include "../rheaCommonLib/rheaThread.h"
#include "SocketBridgeEnumAndDefine.h"

namespace socketbridge
{
	//*************** fwd decalration
	class	Task;
	i16		SocketBridgeTaskThreadFn(void *userParam);

	

	/********************************************************
	 *
	 * TaskStatus
	 *
	 * Questa classe è thread-safe
	 */
	class TaskStatus
	{
	public:
		enum eStatus
		{
			eStatus_pending = 0,
			eStatus_running = 1,
			eStatus_finished = 2
		};

	public:
							TaskStatus ();
							~TaskStatus();

		void				setStatus(eStatus s);
		void				setMessage(const char *format, ...);
		void				setStatusAndMessage(eStatus s, const char *format, ...);

		u32					getUID() const																			{ return uid; }
		void				getStatusAndMesssage(eStatus *out_status, char *out_msg, u32 sizeofmsg);
		u64					getTimeFinishedMSec() const																{ return timeFinishedMSec;  }

	private:
		void				priv_doSetStatusNoCS(eStatus s);

	private:
		static u32			nextUID;

	private:
		OSCriticalSection	cs;
		eStatus				status;
		char				msg[128];
		u64					timeFinishedMSec;
		u32					uid;

	public:
		Task				*_task;
		rhea::Allocator		*_localAllocator;
		char				*params;
		
		static void			FREE(TaskStatus *s)																		{ RHEADELETE(s->_localAllocator, s); }
	};

	

	/*******************************************************
	 *
	 * runTask()
	 *
	 *	Questa fn crea una istanza di un oggetto di tipo <Task>, crea un thread e lancia task->run() all'interno del thread appena creato.
	 *	In caso di errore, ritorna NULL
	 *	In caso di successo, ritorna una istanza di TaskStatus grazie alla quale è possibile conoscere lo stato di esecuzione del Task.
	 *	Quanto il task ha finito, TaskStatus->getStatus() ritorna eFinished; durante l'esecuzione, lo stato è eRunning.
	 *	Quando il task termina, viene automaticamente fatto il free dell'istanza di Task.
	 *	Quando TaskStatus ritorna eFinished, allora è possibile fare il free anche dell'istanza di TaskStatus, usando l'apposita statica TaskStatus::FREE()
	 *
	 *	es:
	 *		rhea::Allocator *allocator = rhea::rhea::memory_getDefaultAllocator();
	 *		TaskStatus *ts = runTask<MyTask>(allocator);
	 *		if (NULL == ts) return; //fallimento
	 *		...
	 *		while (1)
	 *		{
				eStatus s;
				ts->getStatusAndMesssage(&s...);
				if (s == eStatus_finished)
				{
					TaskStatus::FREE(ts);
					return; //fine, il task ha terminato
				}
			}
	 */
					template<class TTask>
	TaskStatus*		runTask (rhea::Allocator *localAllocator, const char *params)
	{
		TaskStatus*	status = RHEANEW(localAllocator, TaskStatus)();
		status->_localAllocator = localAllocator;
		status->_task = RHEANEW(localAllocator, TTask)();
		if (NULL != params)
			status->params = rhea::string::alloc(params);

		rhea::HThread hThread;
		eThreadError err = rhea::thread::create(&hThread, socketbridge::SocketBridgeTaskThreadFn, status, 1024);
		if (err != eThreadError_none)
		{
			RHEADELETE(localAllocator, status->_task);
			RHEADELETE(localAllocator, status);
			return NULL;
		}

		return status;
	}



	/********************************************************
	 *
	 * Task
	 *
	 *	La fn run() viene automaticamente eseguita in un thread nuovo.
	 *	Utilizzare il parametro [status] per impostare eventuali messaggi di stato tramite status->setMessage()
	 *	All'uscita di run(), l'istanza della classe viene automaticamente deleted e lo stato viene messo automaticamente a eFinished
	 */
	class Task
	{
	public:
							Task() { }
		virtual				~Task() { }
		virtual void		run (TaskStatus *status, const char *params) = 0;
	};







} // namespace socketbridge

#endif // _SocketBridgeTask_h_
