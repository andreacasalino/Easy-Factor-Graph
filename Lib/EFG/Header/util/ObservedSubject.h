#pragma once
#ifndef __EFG_SUBJ_H__
#define __EFG_SUBJ_H__

#include <list>
#include <stdlib.h>
#include <memory>
#include <mutex>

namespace EFG::sbj {

#define SYNC_SEMAPHORE std::lock_guard<std::mutex> lk(this->semaphore);

	/*!
	 * \brief An object that can be observed
	 */
	class Subject {
	public:
		virtual ~Subject() { if (this->isObserved()) abort(); };

		inline bool isObserved() const { return this->impl->isObserved(); };

		class Observer;
	protected:

		class SubjectImpl {
		public:
			virtual bool isObserved() = 0;
			virtual void addObserver(Subject::Observer* obsv) = 0;
			virtual void remObserver(Subject::Observer* obsv) = 0;
		};
		Subject(std::unique_ptr<SubjectImpl> impl) : impl(std::move(impl)) {};
		inline SubjectImpl* getImpl() const { return this->impl.get(); };

	private:
		std::unique_ptr<SubjectImpl> impl;
	};

	/*!
	 * \brief An object observing a Subject
	 */
	class Subject::Observer {
	public:
		Observer(const Subject& subject) : observed(&subject) { this->observed->getImpl()->addObserver(this); };
		~Observer(){ this->observed->getImpl()->remObserver(this); };
	private:
	// data
		const Subject*           observed;
	};

	/*!
	 * \brief An object observable by a single subject at a time
	 */
	class SingleObservable : public Subject {
	private:
		class SingleObsvervbleImpl : public SubjectImpl {
		public:
			SingleObsvervbleImpl() : observer(nullptr) {};

			inline bool isObserved() final { 
				SYNC_SEMAPHORE
				return (this->observer != nullptr); 
			};
			void addObserver(Observer* obsv) final { 
				SYNC_SEMAPHORE
				if (this->observer != nullptr) abort(); 
				this->observer = obsv; 
			};
			void remObserver(Observer* obsv) final { 
				SYNC_SEMAPHORE
				this->observer = nullptr; 
			};
		private:
			std::mutex semaphore;
			Observer* observer;
		};
	public:
		SingleObservable() : Subject(std::make_unique<SingleObsvervbleImpl>()) {}
	};

	/*!
	 * \brief An object observable by several subjects at a time
	 */
	class MultiObservable : public Subject {
	private:
		class MultiObservablempl : public SubjectImpl {
		public:
			inline bool isObserved() final { 
				SYNC_SEMAPHORE
				return (!this->observers.empty()); 
			};
			void addObserver(Observer* obsv) final { 
				SYNC_SEMAPHORE
				this->observers.push_back(obsv); 
			};
			void remObserver(Observer* obsv) final { 
				SYNC_SEMAPHORE
				this->observers.remove(obsv); 
			};
		private:
			std::mutex semaphore;
			std::list<Observer*>  	observers;
		};
	public:
		MultiObservable() : Subject(std::make_unique<MultiObservablempl>()) {}
	};

}

#endif