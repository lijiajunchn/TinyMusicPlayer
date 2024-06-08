#include "../head/mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QMessagebox>
#include <random>

CMainWindow::CMainWindow(QWidget *parent):
    QMainWindow(parent),
    m_p_client(new Ui::MainWindow)
{
    m_p_client->setupUi(this);
    m_p_audio_output = new QAudioOutput(this);
    m_p_media_player = new QMediaPlayer(this);
    m_p_media_player->setAudioOutput(m_p_audio_output);
    m_p_audio_output->setVolume(1);
    m_p_client->volumeSlider->setRange(0, 100);
    m_p_client->volumeSlider->setValue(100);
    connect(m_p_client->volumeSlider,
            &QSlider::valueChanged,
            this,
            [=](int value)
            {
        if(0 == value)
            m_p_client->volumeBtn->setIcon(QIcon(":/icon/volume_zero.ico"));
        else if(50 < value)
            m_p_client->volumeBtn->setIcon(QIcon(":/icon/volume_high.ico"));
        else
            m_p_client->volumeBtn->setIcon(QIcon(":/icon/volume_low.ico"));
        m_p_audio_output->setVolume(0.01 * value);
    });
    connect(m_p_media_player,
            &QMediaPlayer::durationChanged,
            this,
            [=](qint64 duration)
            {
        m_p_client->totalLabel->setText(
                    QString("%1:%2").
                    arg(duration/1000/60,2,10,QChar('0')).
                    arg(duration/1000%60,2,10,QChar('0')));
        m_p_client->musicSlider->setRange(0,duration);
    });
    connect(m_p_media_player,
            &QMediaPlayer::positionChanged,
            this,
            [=](qint64 position)
            {
        m_p_client->curLabel->setText(
                    QString("%1:%2").
                    arg(position/1000/60,2,10,QChar('0')).
                    arg(position/1000%60,2,10,QChar('0')));
        m_p_client->musicSlider->setValue(position);
    });
    connect(m_p_client->musicSlider,
            &QSlider::sliderMoved,
            m_p_media_player,
            &QMediaPlayer::setPosition);
    connect(m_p_media_player,
            &QMediaPlayer::mediaStatusChanged,
            this,
            [=](QMediaPlayer::MediaStatus status)
            {
        if(status != QMediaPlayer::EndOfMedia)
            return;
        if(2 == m_play_rule)
            {
            m_p_media_player->play();
            return;
        }
        if(1 == m_play_rule)
            randomIndex();
        else if(0 == m_play_rule)
            m_cur_index = (m_cur_index + 1) % m_play_list.size();
        m_p_client->playList->setCurrentRow(m_cur_index);
        nextMusic();
    });
}

CMainWindow::~CMainWindow()
{
    delete m_p_client;
}

void CMainWindow::on_dirBtn_clicked()
{
    QString music_dir = QFileDialog::
        getExistingDirectory(this, "请选择音乐目录", "Music");
    if(!music_dir.length())
        return;
    if(m_dir_set.contains(music_dir))
    {
        QMessageBox::critical(this, "错误", "目录重复导入！");
        return;
    }
    m_dir_set.insert(music_dir);
    QDir dir(music_dir);
    QFile type_file(":/configure/type_constraints.txt");
    QStringList type_list;
    if (type_file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QTextStream in(&type_file);
        while (!in.atEnd())
        {
            QString type_line = in.readLine();
            type_list << type_line;
        }
        type_file.close();
    }
    QStringList music_list = dir.entryList(type_list);
    m_p_client->playList->addItems(music_list);
    m_p_client->playList->setCurrentRow(0);
    int sum = 0;
    for(auto &file : music_list)
    {
        m_play_list.append(QUrl::fromLocalFile(music_dir+"/"+file));
        ++sum;
    }
    if(0 == sum)
    {
        QMessageBox::warning(this, "警告", "当前目录无音乐！");
        return;
    }
    QMessageBox::information(this, "信息", "成功导入"+QString::number(sum)+"首音乐！");
}

void CMainWindow::on_playBtn_clicked()
{
    if(m_play_list.empty())
        return;
    switch (m_p_media_player->playbackState())
    {
    case QMediaPlayer::PlaybackState::StoppedState:
        m_cur_index = m_p_client->playList->currentRow();
        m_p_media_player->setSource(m_play_list[m_cur_index]);
        m_p_media_player->play();
        m_p_client->playBtn->setIcon(QIcon(":/icon/pause.ico"));
        break;
    case QMediaPlayer::PlaybackState::PlayingState:
        m_p_media_player->pause();
        m_p_client->playBtn->setIcon(QIcon(":/icon/play.ico"));
        break;
    case QMediaPlayer::PlaybackState::PausedState:
        m_p_media_player->play();
        m_p_client->playBtn->setIcon(QIcon(":/icon/pause.ico"));
        break;
    }
}

void CMainWindow::on_preBtn_clicked()
{
    if(m_play_list.empty())
        return;
    if(1 == m_play_rule)
        randomIndex();
    else
        m_cur_index = (m_cur_index - 1) % m_play_list.size();
    m_p_client->playList->setCurrentRow(m_cur_index);
    nextMusic();
}

void CMainWindow::on_nextBtn_clicked()
{
    if(m_play_list.empty())
        return;
    if(1 == m_play_rule)
        randomIndex();
    else
        m_cur_index = (m_cur_index + 1) % m_play_list.size();
    m_p_client->playList->setCurrentRow(m_cur_index);
    nextMusic();
}

void CMainWindow::on_volumeBtn_clicked()
{
    if(0 == m_cur_volume)
    {
        m_p_audio_output->setVolume(0);
        m_cur_volume = m_p_client->volumeSlider->value();
        m_p_client->volumeSlider->setValue(0);
    }
    else
    {
        m_p_audio_output->setVolume(m_cur_volume);
        m_p_client->volumeSlider->setValue(m_cur_volume);
        m_cur_volume = 0;
    }
}

void CMainWindow::on_playList_doubleClicked(const QModelIndex &index)
{
    m_cur_index = index.row();
    nextMusic();
}

void CMainWindow::on_playRuleBox_currentIndexChanged(int index)
{
    m_play_rule = index;
}

void CMainWindow::nextMusic() const
{
    m_p_media_player->setSource(m_play_list[m_cur_index]);
    m_p_media_player->play();
    m_p_client->playBtn->setIcon(QIcon(":/icon/pause.ico"));
}

void CMainWindow::randomIndex()
{
    std::random_device rd;
    std::mt19937 engine(rd());
    std::uniform_int_distribution<> distrib(0, m_play_list.size() - 1);
    m_cur_index = distrib(engine);
}
