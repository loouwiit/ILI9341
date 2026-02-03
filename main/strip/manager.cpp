#include "manager.hpp"

#include "stringCompare.hpp"

bool Strip::Manager::isInited()
{
	return snapshot != nullptr;
}

void Strip::Manager::init()
{
	if (snapshot == nullptr)
		snapshot = new Snapshot{ strip.getCount() };
}

void Strip::Manager::deinit()
{
	if (isStarted())
		stop();

	while (snapshot->next->id != 0) snapshot = snapshot->next;
	while (snapshot->id != 0)
	{
		snapshot = snapshot->last;
		delete snapshot->next;
	}
	delete snapshot;
	snapshot = nullptr;
}

bool Strip::Manager::isStarted()
{
	return stripTask != nullptr;
}

void Strip::Manager::start()
{
	if (!isInited()) return;

	snapshot = snapshot->last;
	snapshotNextChangeTime = 0;
	stripTask = Task::addTask(stripThreadMain, "strip service", this);
}

void Strip::Manager::stop()
{
	if (!isStarted()) return;

	Task::removeTask(stripTask);
	stripTask = nullptr;
}

uint32_t Strip::Manager::load(IFile& file)
{
	if (!file.isOpen()) return 0;

	char buffer[sizeof(FileMagicString)]{};
	file.read(&buffer, sizeof(FileMagicString));
	if (!stringCompare(buffer, sizeof(buffer), FileMagicString, sizeof(FileMagicString)))
	{
		ESP_LOGE(TAG, "magic string don't macth, readed:%s", buffer);
		return 0;
	}

	decltype(strip.ledCount) fileLedCount = 0;
	file.read(&fileLedCount, sizeof(fileLedCount));

	Snapshot fileIn{ fileLedCount };

	while (snapshot->id != 0)
		snapshot = snapshot->next;

	auto* head = snapshot; // head

	decltype(Snapshot::id) count = 0;
	file.read(&count, sizeof(count));
	if (count < 0)
	{
		ESP_LOGE(TAG, "load count = %d < 0!", count);
		return 0;
	}
	ESP_LOGI(TAG, "load count = %d", count);

	decltype(count) i = 0;
	while (i < count)
	{
		file >> fileIn;
		*snapshot = fileIn;
		snapshot->id = i;
		i++;
		if (snapshot->next == head && i != count) add();
		else next();
	}


	return count;
}

uint32_t Strip::Manager::save(OFile& file)
{
	if (!file.isOpen()) return 0;

	file.write(&FileMagicString, sizeof(FileMagicString));
	file.write(&strip.ledCount, sizeof(strip.ledCount));

	auto* pointer = snapshot;
	while (pointer->id != 0)
		pointer = pointer->next;
	auto count = pointer->last->id + 1;

	file.write(&count, sizeof(count));

	file << *pointer;
	pointer = pointer->next;
	while (pointer->id != 0)
	{
		file << *pointer;
		pointer = pointer->next;
	}

	return count;
}

int Strip::Manager::getId()
{
	return snapshot->id;
}

TickType_t Strip::Manager::getLastTime()
{
	return snapshot->lastTime;
}

void Strip::Manager::setLastTime(TickType_t lastTime)
{
	snapshot->lastTime = lastTime;
}

Strip::RGB& Strip::Manager::operator[](uint32_t index)
{
	return (*snapshot)[index];
}

void Strip::Manager::apply()
{
	strip << *snapshot;
	strip.flush();
}

void Strip::Manager::last()
{
	snapshot = snapshot->last;
}

void Strip::Manager::next()
{
	snapshot = snapshot->next;
}

void Strip::Manager::add()
{
	auto* newSnapshot = new Strip::Snapshot{ *snapshot };
	newSnapshot->last = snapshot;
	newSnapshot->next = snapshot->next;
	newSnapshot->next->last = newSnapshot;
	newSnapshot->last->next = newSnapshot;

	newSnapshot->id++;
	for (auto* nowSnapshot = newSnapshot->next; nowSnapshot->id != 0; nowSnapshot = nowSnapshot->next)
		nowSnapshot->id++;

	next();
}

void Strip::Manager::remove()
{
	if (snapshot->id == 0 && snapshot->next->id == 0)
	{
		// 清除但不delete
		*snapshot = Strip::Snapshot{ strip.getCount() };
		return;
	}

	snapshot->last->next = snapshot->next;
	snapshot->next->last = snapshot->last;

	auto* deleteSnapshot = snapshot;
	snapshot = snapshot->next->id != 0 ? snapshot->next : snapshot->last;

	for (auto* nowSnapshot = deleteSnapshot->next; nowSnapshot->id != 0; nowSnapshot = nowSnapshot->next)
		nowSnapshot->id--;
	delete deleteSnapshot;
}

TickType_t Strip::Manager::stripThreadMain(void* param)
{
	auto& self = *(Manager*)param;

	if (!self.isStarted())
		return Task::infinityTime;

	auto nowTime = xTaskGetTickCount();
	if (self.snapshotNextChangeTime < nowTime)
	{
		self.snapshotNextChangeTime = nowTime + self.snapshot->lastTime;
		self.next();
		self.apply();
	}

	TickType_t sleepTime = 0;
	if (nowTime < self.snapshotNextChangeTime)
		sleepTime = self.snapshotNextChangeTime - nowTime;
	else sleepTime = 1; // 最少睡1tick

	if (sleepTime > MaxStripTaskSleepTime)
		sleepTime = MaxStripTaskSleepTime; // 最多睡MaxStripTaskSleepTime

	return sleepTime;
}
