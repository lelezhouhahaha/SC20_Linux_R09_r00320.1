#include "music_select.h"

music_select::music_select()
{
}

QStringList music_select::addSubFolderImages(QString path)
{
	//�ж�·���Ƿ����
	QStringList string_list;
	QDir dir(path);
	if (!dir.exists())
	{
		return string_list;
	}

	//��ȡ��ѡ�ļ����͹�����
	QStringList filters;
	filters << QString("*.mp3") << QString("*.wav");

	//��������������ù�����
	QDirIterator dir_iterator(path, filters, QDir::Files | QDir::NoSymLinks, QDirIterator::Subdirectories);
	while (dir_iterator.hasNext())
	{
		dir_iterator.next();
		QFileInfo file_info = dir_iterator.fileInfo();
		QString absolute_file_path = file_info.absoluteFilePath();
		string_list.append(absolute_file_path);
	}
	return string_list;
}

QString music_select::music_run(QString input)
{
	QString music_str;
	return music_str;
}
