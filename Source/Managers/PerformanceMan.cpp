#include "PerformanceMan.h"
#include "MovableMan.h"
#include "FrameMan.h"
#include "AudioMan.h"

#include "GUI.h"
#include "AllegroBitmap.h"

#include <array>

#include "DebugMan.h"
#include "imgui/imgui.h"

using namespace RTE;

const std::array<std::string, PerformanceMan::PerformanceCounters::PerfCounterCount> PerformanceMan::m_PerfCounterNames = {"Total", "Act AI", "Act Travel", "Act Update", "Prt Travel", "Prt Update", "Activity", "Scripts"};

thread_local std::array<uint64_t, PerformanceMan::PerformanceCounters::PerfCounterCount> s_PerfMeasureStart; //!< Current measurement start time in microseconds.
thread_local std::array<uint64_t, PerformanceMan::PerformanceCounters::PerfCounterCount> s_PerfMeasureStop; //!< Current measurement stop time in microseconds.

PerformanceMan::PerformanceMan() {
	Clear();
}

PerformanceMan::~PerformanceMan() {
	Destroy();
}

void PerformanceMan::Clear() {
	m_ShowPerfStats = false;
	m_AdvancedPerfStats = true;
	m_Sample = 0;
	m_SimUpdateTimer = nullptr;
	m_MSPSUs.clear();
	m_MSPSUAverage = 0;
	m_MSPFs.clear();
	m_MSPFAverage = 0;
	m_MSPUs.clear();
	m_MSPUAverage = 0;
	m_MSPDs.clear();
	m_MSPDAverage = 0;
	m_CurrentPing = 0;
}

void PerformanceMan::Initialize() {
	m_SimUpdateTimer = std::make_unique<Timer>();

	for (int counter = 0; counter < PerformanceCounters::PerfCounterCount; ++counter) {
		for (int i = 0; i < c_MaxSamples; ++i) {
			m_PerfData[counter][i] = 0;
		}
		m_PerfPercentages[counter].fill(0);
	}
}

void PerformanceMan::StartPerformanceMeasurement(PerformanceCounters counter) {
	s_PerfMeasureStart[counter] = g_TimerMan.GetAbsoluteTime();
}

void PerformanceMan::StopPerformanceMeasurement(PerformanceCounters counter) {
	s_PerfMeasureStop[counter] = g_TimerMan.GetAbsoluteTime();
	AddPerformanceSample(counter, s_PerfMeasureStop[counter] - s_PerfMeasureStart[counter]);
}

void PerformanceMan::NewPerformanceSample() {
	m_Sample++;
	if (m_Sample >= c_MaxSamples) {
		m_Sample = 0;
	}

	for (int counter = 0; counter < PerformanceCounters::PerfCounterCount; ++counter) {
		m_PerfData[counter][m_Sample] = 0;
		m_PerfPercentages[counter][m_Sample] = 0;
	}
}

void PerformanceMan::CalculateSamplePercentages() {
	for (int counter = 0; counter < PerformanceCounters::PerfCounterCount; ++counter) {
		int samplePercentage = static_cast<int>(static_cast<float>(m_PerfData[counter][m_Sample]) / static_cast<float>(m_PerfData[counter][PerformanceCounters::SimTotal]) * 100);
		m_PerfPercentages[counter][m_Sample] = samplePercentage;
	}
}

uint64_t PerformanceMan::GetPerformanceCounterAverage(PerformanceCounters counter) const {
	uint64_t totalPerformanceMeasurement = 0;
	int sample = m_Sample;
	for (int i = 0; i < c_Average; ++i) {
		totalPerformanceMeasurement += m_PerfData[counter][sample];
		sample--;
		if (sample < 0) {
			sample = c_MaxSamples - 1;
		}
	}
	return totalPerformanceMeasurement / c_Average;
}

void PerformanceMan::CalculateTimeAverage(std::deque<float>& timeMeasurements, float& avgResult, float newTimeMeasurement) const {
	timeMeasurements.emplace_back(newTimeMeasurement);
	while (timeMeasurements.size() > c_MSPAverageSampleSize) {
		timeMeasurements.pop_front();
	}
	avgResult = 0;
	for (const float& timeMeasurement: timeMeasurements) {
		avgResult += timeMeasurement;
	}
	avgResult /= static_cast<float>(timeMeasurements.size());
}

void PerformanceMan::UpdateMSPF(long long measuredUpdateTime, long long measuredDrawTime) {
	CalculateTimeAverage(m_MSPUs, m_MSPUAverage, static_cast<float>(measuredUpdateTime / 1000));
	CalculateTimeAverage(m_MSPDs, m_MSPDAverage, static_cast<float>(measuredDrawTime / 1000));
	CalculateTimeAverage(m_MSPFs, m_MSPFAverage, static_cast<float>((measuredUpdateTime + measuredDrawTime) / 1000));
}

void PerformanceMan::Draw(BITMAP* bitmapToDrawTo) {
	if (m_ShowPerfStats) {
		AllegroBitmap drawBitmap(bitmapToDrawTo);

		GUIFont* guiFont = g_FrameMan.GetLargeFont(true);
		char str[128];

		float fps = 1.0F / (m_MSPFAverage / 1000.0F);
		float ups = 1.0F / (m_MSPSUAverage / 1000.0F);
		std::snprintf(str, sizeof(str), "FPS: %.0f | UPS: %.0f", fps, ups);
		guiFont->DrawAligned(&drawBitmap, c_StatsOffsetX, c_StatsHeight, str, GUIFont::Left);

		std::snprintf(str, sizeof(str), "Frame: %.1fms | Update: %.1fms | Draw: %.1fms", m_MSPFAverage, m_MSPUAverage, m_MSPDAverage);
		guiFont->DrawAligned(&drawBitmap, c_StatsOffsetX, c_StatsHeight + 10, str, GUIFont::Left);

		std::snprintf(str, sizeof(str), "Time Scale: x%.2f ([1]-, [2]+, [RAlt+1]Rst) | Sim Speed: x%.2f", g_TimerMan.GetTimeScale(), g_TimerMan.GetSimSpeed());
		guiFont->DrawAligned(&drawBitmap, c_StatsOffsetX, c_StatsHeight + 20, str, GUIFont::Left);

		float deltaTime = g_TimerMan.GetDeltaTimeMS();
		std::snprintf(str, sizeof(str), "DeltaTime: %.2f ms ([5]-, [6]+, [RAlt+5]Rst)", deltaTime);
		guiFont->DrawAligned(&drawBitmap, c_StatsOffsetX, c_StatsHeight + 30, str, GUIFont::Left);

		std::snprintf(str, sizeof(str), "Actors: %li", g_MovableMan.GetActorCount());
		guiFont->DrawAligned(&drawBitmap, c_StatsOffsetX, c_StatsHeight + 40, str, GUIFont::Left);

		std::snprintf(str, sizeof(str), "Particles: %li", g_MovableMan.GetParticleCount());
		guiFont->DrawAligned(&drawBitmap, c_StatsOffsetX, c_StatsHeight + 50, str, GUIFont::Left);

		std::snprintf(str, sizeof(str), "Objects: %i", g_MovableMan.GetKnownObjectsCount());
		guiFont->DrawAligned(&drawBitmap, c_StatsOffsetX, c_StatsHeight + 60, str, GUIFont::Left);

		std::snprintf(str, sizeof(str), "MOIDs: %i", g_MovableMan.GetMOIDCount());
		guiFont->DrawAligned(&drawBitmap, c_StatsOffsetX, c_StatsHeight + 70, str, GUIFont::Left);

		if (int totalPlayingChannelCount = 0, realPlayingChannelCount = 0; g_AudioMan.GetPlayingChannelCount(&totalPlayingChannelCount, &realPlayingChannelCount)) {
			std::snprintf(str, sizeof(str), "Sound Channels: %d / %d Real | %d / %d Virtual", realPlayingChannelCount, g_AudioMan.GetTotalRealChannelCount(), totalPlayingChannelCount - realPlayingChannelCount, g_AudioMan.GetTotalVirtualChannelCount());
		}
		guiFont->DrawAligned(&drawBitmap, c_StatsOffsetX, c_StatsHeight + 80, str, GUIFont::Left);

		if (!m_SortedScriptTimings.empty()) {
			std::snprintf(str, sizeof(str), "Lua scripts taking the most time to call Update() this frame:");
			guiFont->DrawAligned(&drawBitmap, c_StatsOffsetX, c_StatsHeight + 100, str, GUIFont::Left);

			for (int i = 0; i < std::min((size_t)3, m_SortedScriptTimings.size()); i++) {
				std::pair<std::string, ScriptTiming> scriptTiming = m_SortedScriptTimings.at(i);

				std::snprintf(str, sizeof(str), "%.1fms total with %i calls in %s", scriptTiming.second.m_Time / 1000.0, scriptTiming.second.m_CallCount, scriptTiming.first.c_str());
				guiFont->DrawAligned(&drawBitmap, c_StatsOffsetX, c_StatsHeight + 110 + i * 10, str, GUIFont::Left);
			}
		}

		if (m_AdvancedPerfStats) {
			DrawPeformanceGraphs(drawBitmap);
		}
	}
}

void PerformanceMan::DrawPeformanceGraphs(AllegroBitmap& bitmapToDrawTo) {
	CalculateSamplePercentages();

	GUIFont* guiFont = g_FrameMan.GetLargeFont(true);
	char str[128];

	for (int pc = 0; pc < PerformanceCounters::PerfCounterCount; ++pc) {
		int blockStart = c_GraphsStartOffsetY + pc * c_GraphBlockHeight;

		guiFont->DrawAligned(&bitmapToDrawTo, c_StatsOffsetX, blockStart, m_PerfCounterNames[pc], GUIFont::Left);

		int perc = static_cast<int>((static_cast<float>(GetPerformanceCounterAverage(static_cast<PerformanceCounters>(pc))) / static_cast<float>(GetPerformanceCounterAverage(PerformanceCounters::SimTotal)) * 100));
		std::snprintf(str, sizeof(str), "%%: %u", perc);
		guiFont->DrawAligned(&bitmapToDrawTo, c_StatsOffsetX + 60, blockStart, str, GUIFont::Left);

		// Print average processing time in milliseconds.
		std::snprintf(str, sizeof(str), "T: %llu", GetPerformanceCounterAverage(static_cast<PerformanceCounters>(pc)) / 1000);
		guiFont->DrawAligned(&bitmapToDrawTo, c_StatsOffsetX + 96, blockStart, str, GUIFont::Left);

		int graphStart = blockStart + c_GraphsOffsetX;

		// Draw graph backgrounds.
		bitmapToDrawTo.DrawRectangle(c_StatsOffsetX, graphStart, c_MaxSamples, c_GraphHeight, makecol32(96, 19, 32), true); // Palette index 240.
		bitmapToDrawTo.DrawLine(c_StatsOffsetX, graphStart + c_GraphHeight / 2, c_StatsOffsetX - 1 + c_MaxSamples, graphStart + c_GraphHeight / 2, makecol32(200, 206, 140)); // Palette index 96.

		// Draw sample dots.
		int peak = 0;
		int sample = m_Sample;
		for (int i = 0; i < c_MaxSamples; ++i) {
			// Show microseconds in graphs, assume that the RealToSimCap is the highest value on the graph. The graph will scale with the RealToSimCap if it is changed.
			int value = std::clamp(static_cast<int>(static_cast<float>(m_PerfData[pc][sample]) / (g_TimerMan.GetRealToSimCap() * 1000000.0F) * 100.0F), 0, 100);
			int dotHeight = static_cast<int>(static_cast<float>(c_GraphHeight) / 100.0F * static_cast<float>(value));

			bitmapToDrawTo.SetPixel(c_StatsOffsetX - 1 + c_MaxSamples - i, graphStart + c_GraphHeight - dotHeight, makecol32(234, 21, 7)); // Palette index 13.

			if (peak < m_PerfData[pc][sample]) {
				peak = static_cast<int>(m_PerfData[pc][sample]);
			}

			if (sample == 0) {
				sample = c_MaxSamples;
			}
			sample--;
		}

		// Print peak values
		guiFont->DrawAligned(&bitmapToDrawTo, c_StatsOffsetX + 130, blockStart, "Peak: " + std::to_string(peak / 1000), GUIFont::Left);
	}
}

void PerformanceMan::DrawCurrentPing() const {
	AllegroBitmap allegroBitmap(g_FrameMan.GetBackBuffer8());
	g_FrameMan.GetLargeFont()->DrawAligned(&allegroBitmap, g_FrameMan.GetBackBuffer8()->w - 25, g_FrameMan.GetBackBuffer8()->h - 14, "PING: " + std::to_string(m_CurrentPing), GUIFont::Right);
}

void PerformanceMan::ImGui() {
	CalculateSamplePercentages();
	if (ImGui::Begin("Performance Statistics")) {
		float fps = 1.0F / (m_MSPFAverage / 1000.0F);
		float ups = 1.0F / (m_MSPSUAverage / 1000.0F);
		ImGui::Text("FPS: %.0f UPS: %.0f", fps, ups);
		ImGui::Text("Frame: %.2f | Update: %.1f | Draw: %.1f", m_MSPFAverage, m_MSPUAverage, m_MSPDAverage);
		float timeScale = g_TimerMan.GetTimeScale();
		ImGui::Text("TimeScale: "); ImGui::SameLine();
		if (ImGui::InputFloat("##ts", &timeScale, 0.1f, 1.0f, "%.6f")) {
			if (timeScale < 0) {
				timeScale = 0;
			}
			g_TimerMan.SetTimeScale(timeScale);
		}

		float deltaTime = g_TimerMan.GetDeltaTimeSecs();
		ImGui::Text("DeltaTime: "); ImGui::SameLine();
		if (ImGui::InputFloat("##dt", &deltaTime, 0.0001f, 0.001f, "%.6f")) {
			if (deltaTime < 0) {
				deltaTime = 0;
			}
			g_TimerMan.SetDeltaTimeSecs(deltaTime);
		}

		ImGui::Text("Actors: %li", g_MovableMan.GetActorCount());
		ImGui::Text("Particles: %li", g_MovableMan.GetParticleCount());
		ImGui::Text("Objects: %i", g_MovableMan.GetKnownObjectsCount());
		ImGui::Text("MOIDs: %i", g_MovableMan.GetMOIDCount());


		if (int totalPlayingChannelCount = 0, realPlayingChannelCount = 0; g_AudioMan.GetPlayingChannelCount(&totalPlayingChannelCount, &realPlayingChannelCount)) {
			ImGui::Text("Sound Channels: %d / %d Real | %d / %d Virtual", realPlayingChannelCount, g_AudioMan.GetTotalRealChannelCount(), totalPlayingChannelCount - realPlayingChannelCount, g_AudioMan.GetTotalVirtualChannelCount());
		}

		if (!m_SortedScriptTimings.empty()) {
			ImGui::Text("ScriptTimings");
			if (ImGui::BeginChild("script_timings", ImVec2(0, ImGui::GetFontSize() * 5))) {
				for (auto& [script, timing]: m_SortedScriptTimings) {
					ImGui::Text("%.1fms | %i calls from: %s", timing.m_Time / 1000.0f, timing.m_CallCount, script.c_str());
				}
			}
			ImGui::EndChild();
		}

		if (ImGui::TreeNode("Preformance Graphs")) {
			for (int pc = 0; pc < PerformanceCounters::PerfCounterCount; pc++) {
				ImGui::PushID(pc);
				if (ImGui::TreeNode(m_PerfCounterNames[pc].c_str())) {
					auto& perfData = m_PerfData[pc];
					std::array<float, c_MaxSamples> floatData;
					int peak = 0;
					int sample = m_Sample;
					for (int i = 0; i < c_MaxSamples; ++i) {
						if (peak < perfData.at(i)) {
							peak = perfData.at(i);
						}
						floatData[i] = perfData[sample];
						if (sample == 0) {
							sample = c_MaxSamples;
						}
						sample--;
					}
					ImGui::PushID(pc);
					ImGui::Text("Peak: %i", peak);
					ImGui::SameLine();
					int perc = static_cast<int>((static_cast<float>(GetPerformanceCounterAverage(static_cast<PerformanceCounters>(pc))) / static_cast<float>(GetPerformanceCounterAverage(PerformanceCounters::SimTotal)) * 100));
					ImGui::Text("%%: %i", perc);
					ImGui::SameLine();
					ImGui::Text("Average Time: %lu", GetPerformanceCounterAverage(static_cast<PerformanceCounters>(pc)) / 1000);
					ImGui::PlotHistogram("", floatData.data(), c_MaxSamples, 0, nullptr, 0.0f, g_TimerMan.GetRealToSimCap() * 1E6f);
					ImGui::PopID();

					ImGui::TreePop();
				}
				ImGui::PopID();
			}
			ImGui::TreePop();
		}
	}
	ImGui::End();
}

void PerformanceMan::UpdateSortedScriptTimings(const std::unordered_map<std::string, ScriptTiming>& scriptTimings) {
	std::vector<std::pair<std::string, ScriptTiming>> sortedScriptTimings;
	for (auto it = scriptTimings.begin(); it != scriptTimings.end(); it++) {
		sortedScriptTimings.push_back(*it);
	}

	std::sort(sortedScriptTimings.begin(), sortedScriptTimings.end(), [](const auto& l, const auto& r) { return l.second.m_Time > r.second.m_Time; });

	g_PerformanceMan.m_SortedScriptTimings = sortedScriptTimings;
}
