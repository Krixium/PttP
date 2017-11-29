#pragma once

#include <string>
#include <fstream>
#include <memory>

#include <QByteArray>
#include <QFileDialog>
#include <QWidget>

using namespace std;

class FileManip : public QWidget 
{
	Q_OBJECT

public:
	FileManip(QObject* parent = nullptr);
	~FileManip();

	QByteArray GetNextBytes();
	QByteArray GetPreviousBytes();

private:
	string mFile;
	unique_ptr<ifstream> mInStream;
	char mBuffer[512];

public slots:
	void SelectFile();

signals:
	void fileChanged(string newFileName);
};
