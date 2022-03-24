// /**
//  * Author:    Andrea Casalino
//  * Created:   01.01.2021
//  *
//  * report any bug to andrecasa91@gmail.com.
//  **/

// #include <EasyFactorGraph/structure/BeliefPropagator.h>

// #include "BeliefPropagationUtils.h"

// #include <algorithm>
// #include <list>
// #include <omp.h>

// namespace EFG::strct {
// namespace {
// void resetMessages(const std::vector<HiddenCluster> &clusters) {
//   for (const auto &cluster : clusters) {
//     for (auto *node : cluster.nodes) {
//       for (auto &[connected_node, connection] : node->active_connections) {
//         connection.message.reset();
//       }
//     }
//   }
// }

// bool message_passing(HiddenCluster &cluster, const PropagationKind &kind) {
//   std::vector<std::vector<ConnectionAndDependencies *>> jobs;

//   std::list<ConnectionAndDependencies *> open_set;
//   for (auto &element : *cluster.connectivity) {
//     open_set.push_back(&element);
//   }
//   while (!open_set.empty()) {
//     to_compute;

// #pragma omp for
//     for (int k = 0; k < to_compute.size(); ++k) {
//     }
//   }
//   return true;
// }

// bool loopy_propagation(HiddenCluster &cluster, const PropagationKind &kind,
//                        const std::size_t max_iterations);
// } // namespace

// PropagationResult
// BeliefPropagator::propagateBelief(const PropagationKind &kind) {
//   if (hasPropagationResult() &&
//       (kind == getLastPropagationResult().propagation_kind_done)) {
//     return;
//   }
//   auto &clusters = getClusters_();
//   resetMessages(clusters);
//   PropagationResult result;
//   result.was_completed = true;
//   result.propagation_kind_done = kind;
//   for (auto &cluster : clusters) {
//     if (nullptr == cluster.connectivity) {
//       update_connectivity(cluster);
//     }
//     if (message_passing(cluster, kind)) {
//       continue;
//     }
//     if (!loopy_propagation(
//             cluster, kind,
//             getPropagationContext().max_iterations_loopy_propagation)) {
//       result.was_completed = false;
//     }
//   }
//   return result;
// }

// void BeliefPropagator::propagateBelief(const PropagationKind &kind) {
//   if ((nullptr != this->lastPropagation) &&
//       (kind == this->lastPropagation->kindDone)) {
//     return;
//   }
//   // empty all messages
//   std::for_each(this->hidden.clusters.begin(), this->hidden.clusters.end(),
//                 [](const std::set<Node *> &c) {
//                   std::for_each(c.begin(), c.end(),
//                                 [](Node *n) { resetMessages(*n); });
//                 });
//   // do new propagation
//   this->lastPropagation = std::make_unique<PropagationResult>();
//   this->lastPropagation->kindDone = kind;
//   this->lastPropagation->wasTerminated = true;
// #ifdef THREAD_POOL_ENABLED
//   if (nullptr != this->threadPool) {
//     std::for_each(this->hidden.clusters.begin(), this->hidden.clusters.end(),
//                   [&](const std::set<Node *> &cluster) {
//                     if (this->messagePassingThreadPool(cluster, kind)) {
//                       return;
//                     }
//                     if (this->loopyPropagationThreadPool(cluster, kind)) {
//                       return;
//                     }
//                     this->lastPropagation->wasTerminated = false;
//                   });
//   } else {
// #endif
//     std::for_each(this->hidden.clusters.begin(), this->hidden.clusters.end(),
//                   [&](const std::set<Node *> &cluster) {
//                     if (this->messagePassing(cluster, kind)) {
//                       return;
//                     }
//                     if (this->loopyPropagation(cluster, kind)) {
//                       return;
//                     }
//                     this->lastPropagation->wasTerminated = false;
//                   });
// #ifdef THREAD_POOL_ENABLED
//   }
// #endif
// }

// class MessageComputer {
// public:
//   MessageComputer(Node *sender, Node *receiver, const PropagationKind &kind)
//       : kind(kind), sender(sender),
//         recipient(&receiver->activeConnections.find(sender)->second) {
//     for (auto it = sender->activeConnections.begin();
//          it != sender->activeConnections.end(); ++it) {
//       this->dependencies.push_back(&it->second);
//     }
//     this->dependencies.remove(
//         &sender->activeConnections.find(receiver)->second);
//   };
//   MessageComputer(const MessageComputer &) = default;

//   inline bool isComputationPossible() const {
//     for (auto it = this->dependencies.begin(); it !=
//     this->dependencies.end();
//          ++it) {
//       if (nullptr == (*it)->message2This) {
//         return false;
//       }
//     }
//     return true;
//   };

//   // return difference
//   float compute() {
//     std::unique_ptr<distribution::Distribution> newMessage;
//     // gather messages
//     std::set<const distribution::Distribution *> toMerge = {
//         this->recipient->factor.get()};
//     gatherUnaries(toMerge, *this->sender);
//     std::for_each(this->dependencies.begin(), this->dependencies.end(),
//                   [&toMerge](const Connection *c) {
//                     toMerge.emplace(c->message2This.get());
//                   });
//     if (PropagationKind::Sum == kind) {
//       if (1 == toMerge.size()) {
//         newMessage =
//         std::make_unique<distribution::factor::cnst::MessageSum>(
//             **toMerge.begin(),
//             std::set<categoric::VariablePtr>{sender->variable});
//       } else {
//         newMessage =
//         std::make_unique<distribution::factor::cnst::MessageSum>(
//             distribution::factor::cnst::Factor(toMerge),
//             std::set<categoric::VariablePtr>{sender->variable});
//       }
//     } else {
//       if (1 == toMerge.size()) {
//         newMessage =
//         std::make_unique<distribution::factor::cnst::MessageMAP>(
//             **toMerge.begin(),
//             std::set<categoric::VariablePtr>{sender->variable});
//       } else {
//         newMessage =
//         std::make_unique<distribution::factor::cnst::MessageMAP>(
//             distribution::factor::cnst::Factor(toMerge),
//             std::set<categoric::VariablePtr>{sender->variable});
//       }
//     }
//     float difference = 0.f;
//     if (nullptr == this->recipient->message2This) {
//       difference = MAX_DIFF;
//     } else {
//       auto itOld = this->recipient->message2This->getIterator();
//       auto itNew = newMessage->getIterator();
//       if (itOld.getNumberOfValues() != itNew.getNumberOfValues()) {
//         difference = MAX_DIFF;
//       } else {
//         iterator::forEach(
//             itOld, [&itNew, &difference](
//                        const distribution::DistributionIterator &itOld) {
//               difference += fabsf(itNew.getImageRaw() - itOld.getImageRaw());
//               ++itNew;
//             });
//       }
//     }
//     this->recipient->message2This = std::move(newMessage);
//     return difference;
//   };

//   inline const std::list<const Connection *> &getDependencies() const {
//     return this->dependencies;
//   };
//   inline const Connection *getRecipient() const { return this->recipient; };

// private:
//   PropagationKind kind;
//   Node *sender;
//   Connection *recipient;
//   std::list<const Connection *> dependencies;
// };

// typedef std::pair<Node *, Node *> SenderReceiver;

// std::list<SenderReceiver> getOpenSet(const std::set<Node *> &cluster) {
//   std::list<SenderReceiver> openSet;
//   std::for_each(cluster.begin(), cluster.end(), [&openSet](Node *n) {
//     for (auto itA = n->activeConnections.begin();
//          itA != n->activeConnections.end(); ++itA) {
//       openSet.emplace_back(std::make_pair(n, itA->first));
//     }
//   });
//   return openSet;
// }

// void setOnesMessages(const std::set<Node *> &cluster) {
//   std::for_each(cluster.begin(), cluster.end(), [](Node *n) {
//     for (auto itA = n->activeConnections.begin();
//          itA != n->activeConnections.end(); ++itA) {
//       auto mexOnes = std::make_unique<distribution::factor::modif::Factor>(
//           std::set<categoric::VariablePtr>{n->variable});
//       mexOnes->setAllImagesRaw(1.f);
//       itA->second.message2This = std::move(mexOnes);
//     }
//   });
// }

// bool BeliefPropagator::messagePassing(const std::set<Node *> &cluster,
//                                       const PropagationKind &kind) {
//   auto openSet = getOpenSet(cluster);
//   bool progressWasMade;
//   while (!openSet.empty()) {
//     progressWasMade = false;
//     auto it = openSet.begin();
//     while (it != openSet.end()) {
//       MessageComputer computer(it->first, it->second, kind);
//       if (computer.isComputationPossible()) {
//         computer.compute();
//         it = openSet.erase(it);
//         progressWasMade = true;
//       } else
//         ++it;
//     }
//     if (!progressWasMade) {
//       return false;
//     }
//   }
//   return true;
// }

// bool BeliefPropagator::loopyPropagation(const std::set<Node *> &cluster,
//                                         const PropagationKind &kind) {
//   // set to ones all messages
//   setOnesMessages(cluster);
//   // calibrate messages
//   float variationMax, variation;
//   for (std::size_t k = 0; k < this->maxIterationsLoopyPropagtion; ++k) {
//     variationMax = 0.f;
//     std::for_each(cluster.begin(), cluster.end(), [&](Node *n) {
//       for (auto itA = n->activeConnections.begin();
//            itA != n->activeConnections.end(); ++itA) {
//         variation = MessageComputer(n, itA->first, kind).compute();
//         if (variation > variationMax) {
//           variationMax = variation;
//         }
//       }
//     });
//     if (0.f == variationMax) {
//       return true;
//     }
//   }
//   return false;
// }

// #ifdef THREAD_POOL_ENABLED
// bool BeliefPropagator::messagePassingThreadPool(const std::set<Node *>
// &cluster,
//                                                 const PropagationKind &kind)
//                                                 {
//   auto openSet = getOpenSet(cluster);
//   bool progressWasMade;
//   while (!openSet.empty()) {
//     progressWasMade = false;
//     auto it = openSet.begin();
//     std::list<MessageComputer> computers;
//     while (it != openSet.end()) {
//       computers.emplace_back(it->first, it->second, kind);
//       MessageComputer *cmpPtr = &computers.back();
//       if (cmpPtr->isComputationPossible()) {
//         this->threadPool->push([cmpPtr]() { cmpPtr->compute(); });
//         it = openSet.erase(it);
//         progressWasMade = true;
//       } else {
//         computers.pop_back();
//         ++it;
//       }
//     }
//     this->threadPool->wait();
//     if (!progressWasMade) {
//       return false;
//     }
//   }
//   return true;
// }

// bool BeliefPropagator::loopyPropagationThreadPool(
//     const std::set<Node *> &cluster, const PropagationKind &kind) {
//   // set to ones all messages
//   setOnesMessages(cluster);
//   // determine thread safe messages computation order
//   std::list<std::list<MessageComputer>> order;
//   {
//     // get complete list of messages
//     auto toCalibrate = getOpenSet(cluster);
//     auto openSet = toCalibrate;
//     bool calibrationPossible;
//     while (!openSet.empty()) {
//       order.push_back({});
//       std::set<const Connection *> lockedMessages;
//       auto itOp = openSet.begin();
//       while (itOp != openSet.end()) {
//         const Connection *mex =
//             &itOp->second->activeConnections.find(itOp->first)->second;
//         if (lockedMessages.find(mex) == lockedMessages.end()) {
//           order.back().emplace_back(itOp->first, itOp->second, kind);
//           calibrationPossible = true;
//           // check that all dependencies will be not calibrated during this
//           // iteration
//           for (auto dep = order.back().back().getDependencies().begin();
//                dep != order.back().back().getDependencies().end(); ++dep) {
//             if (lockedMessages.find(*dep) != lockedMessages.end()) {
//               calibrationPossible = false;
//               break;
//             }
//           }
//           if (calibrationPossible) {
//             itOp = openSet.erase(itOp);
//             lockedMessages.emplace(order.back().back().getRecipient());
//             for (auto dep = order.back().back().getDependencies().begin();
//                  dep != order.back().back().getDependencies().end(); ++dep) {
//               lockedMessages.emplace(*dep);
//             }
//           } else {
//             order.back().pop_back();
//             ++itOp;
//           }
//         } else {
//           ++itOp;
//         }
//       }
//     }
//   }
//   // calibrate messages
//   float variationMax;
//   std::mutex variationMtx;
//   for (std::size_t k = 0; k < this->maxIterationsLoopyPropagtion; ++k) {
//     variationMax = 0.f;
//     std::for_each(
//         order.begin(), order.end(), [&](std::list<MessageComputer> &manche) {
//           std::for_each(manche.begin(), manche.end(), [&](MessageComputer &c)
//           {
//             MessageComputer *cPtr = &c;
//             this->threadPool->push([cPtr, &variationMtx, &variationMax]() {
//               float variation = cPtr->compute();
//               std::lock_guard<std::mutex> variationLck(variationMtx);
//               if (variation > variationMax) {
//                 variationMax = variation;
//               }
//             });
//           });
//           this->threadPool->wait();
//         });
//     if (0.f == variationMax) {
//       return true;
//     }
//   }
//   return false;
// }
// #endif
// } // namespace EFG::strct
