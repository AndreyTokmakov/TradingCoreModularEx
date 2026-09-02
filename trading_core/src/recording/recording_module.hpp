/**============================================================================
Name        : recording_module.hpp
Created on  : 25.08.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : Records market events on the recording thread.
============================================================================**/

#ifndef FINANCETECHNOLOGYPROJECTS_RECORDING_MODULE_HPP
#define FINANCETECHNOLOGYPROJECTS_RECORDING_MODULE_HPP

#include "queue.hpp"
#include "recorder.hpp"
#include "config.hpp"
#include "recording_event.hpp"
#include "worker.hpp"

namespace trading::recording
{
    class RecordingModule final: public common::Worker<RecordingModule> {
    public:
        RecordingModule(const config::RecordingConfig& recorderConfig,
                        concurrency::Queue<RecordingEvent>& recordingQueue) noexcept;

        void run() const;

    private:

        void process(const market_data::MarketEvent& event) const;
        void process(const execution::ExecutionReport& report) const;
        void process(const execution::OrderRequest& orderRequest) const;

    private:

        std::unique_ptr<IRecorder> recorder;
        concurrency::Queue<RecordingEvent>& recordingQueue;
    };
}

#endif //FINANCETECHNOLOGYPROJECTS_RECORDING_MODULE_HPP