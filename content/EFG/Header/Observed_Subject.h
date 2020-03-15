#pragma once
#ifndef __CRF_OBSV_H__
#define __CRF_OBSV_H__

#include <list>
#include <stdlib.h>


namespace EFG {

	class Subject {
	public:
		Subject() {};

		class Observer {
		public:
			Observer(Subject* to_observe, void* involved) {
				this->observed = to_observe;
				this->observed->Add_to_registered(involved);
				this->observer = involved;
			};
			~Observer() {
				if(this->observed != NULL) this->observed->observers.remove(observer);
			};
			void change_observed(Subject* new_observed) {
				this->observed->observers.remove(observer);
				this->observed = new_observed;
				this->observed->Add_to_registered(this->observer);
			};
			void detach() {
				this->observed->observers.remove(observer);
				this->observed = NULL;
			}
		private:
		// data
			Subject*           observed;
			void*		       observer;
		};

		~Subject() { if (!this->observers.empty()) abort(); };
		size_t Get_observers_number() { return this->observers.size(); };

		const std::list<void*>& Get_observers() { return this->observers; };
	protected:
		virtual void			Add_to_registered(void* observer) { this->observers.push_back(observer); };
	// data
		std::list<void*>		observers;
	};


	class Subject_singleObserver : public Subject {
	private:
		virtual void			Add_to_registered(void* observer) {
			if (!this->observers.empty()) throw 0;
			this->observers.push_back(observer);
		};
	};

}

#endif