/**============================================================================
Name        : recording_module.cpp
Created on  : 25.08.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : Records market events on the recording thread.
============================================================================**/

#include "recording_module.hpp"
#include "trade_recorder.hpp"

namespace trading::recording
{
    RecordingModule::RecordingModule(const config::RecordingConfig&,
                                     concurrency::Queue<RecordingEvent>& recordingQueue,
                                     const common::RuntimeContext&) noexcept:
        recorder { std::make_unique<TradeRecorder>() },
        recordingQueue { recordingQueue }
    {
    }

    void RecordingModule::run() const
    {
        RecordingEvent event;

        while (recordingQueue.waitPop(event))
        {
            std::visit([this](const auto& item) {
                process(item);
            }, event);
        }
    }

    void RecordingModule::process(const market_data::MarketEvent& event) const {
        recorder->record(event);
    }

    void RecordingModule::process(const execution::ExecutionReport& report) const {
        recorder->record(report);
    }

    void RecordingModule::process(const execution::OrderRequest&) const {
    }

}