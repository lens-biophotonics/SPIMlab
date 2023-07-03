#include <QThread>

class AlignmentThread : public QThread
{
    Q_OBJECT

public: 
    explicit AlignmentThread(const cv::Mat& sourceImage1, const cv::Mat& sourceImage2,
                             rapid_af::AlignOptions options)
        : sourceImage1(sourceImage1), sourceImage2(sourceImage2), options(options)
    {
    }
      void run() override
    {
        try {
            shift = rapid_af::align(sourceImage1, sourceImage2, options, &ok);
        } catch (cv::Exception& e) {
            qWarning() << e.what();
            throw std::runtime_error("OpenCV exception (see log)");
        }

        if (!ok) {
            throw std::runtime_error("Agreement threshold not met");
        }
    }

    rapid_af::ShiftResult getShiftResult() const
    {
        return shift;
    }

private:
    void run() override
    {
        try {
            shift = rapid_af::align(sourceImage1, sourceImage2, options, &ok);
        } catch (cv::Exception& e) {
            qWarning() << e.what();
            throw std::runtime_error("OpenCV exception (see log)");
        }

        if (!ok) {
            throw std::runtime_error("Agreement threshold not met");
        }
    }

    cv::Mat sourceImage1;
    cv::Mat sourceImage2;

    rapid_af::AlignOptions options;
    rapid_af::ImageQualityOptions iqOptions;

    rapid_af::ShiftResult shift;
    bool ok = false;
};

int main()
{    
    sourceImages = autofocus->getRois()
    
    rapid_af::AlignOptions options; // Initialize your alignment options
    bool ok1 = false
    bool ok2= false
    AlignmentThread thread1 (sourceImages[0][0], sourceImages[0][1], options);
    AlignmentThread thread2(sourceImages[1][0], sourceImages[1][1], options);
    thread1.start(); 
    thread2.start();
    {
        try {
        shift1 = rapid_af::align(sourceImages[0][0], sourceImages[0][1], options, &ok1);
    } catch (cv::Exception e) {
        logger->warning(e.what());
        throw std::runtime_error("OpenCV exception (see log)");
    }
        
        try {
        shift2 = rapid_af::align(sourceImages[1][0], sourceImages[1][1], options, &ok2);
    } catch (cv::Exception e) {
        logger->warning(e.what());
        throw std::runtime_error("OpenCV exception (see log)");
    }
        
    thread1.start(); 
    thread2.start();

    return 0;}


    }
    AlignmentThread thread2(sourceImages[1][0], sourceImages[1][1], options);
    AlignmentThread thread3(sourceImages[2][0], sourceImages[2][1], options);
    AlignmentThread thread4(sourceImages[3][0], sourceImages[3][1], options);

    thread1.start();
    thread2.start();
    thread3.start();
    thread4.start();

    thread1.wait();
    thread2.wait();
    thread3.wait();
    thread4.wait();

    rapid_af::ShiftResult shift1 = thread1.getShiftResult();
    rapid_af::ShiftResult shift2 = thread2.getShiftResult();
    rapid_af::ShiftResult shift3 = thread3.getShiftResult();
    rapid_af::ShiftResult shift4 = thread4.getShiftResult();

    // Process the shift results...

    return 0;
}




























  void alignment(img soura1, img soura2,options,bool){
    try {
        shift = rapid_af::align(soura1, soura2, options, &ok);
    } catch (cv::Exception e) {
        logger->warning(e.what());
        throw std::runtime_error("OpenCV exception (see log)");
    }
    if (!ok) {
        throw std::runtime_error("Agreement threshold not met");
    }

    return shift.x;
    
  }}
int main(){
  alignmentThread shift1;
  alignmentThread shift2;
  alignmentThread shift3;
  alignmentThread shift4;
  
  alignment = new Align();
  QThread *thread = new QThread();
  thread->setObjectName("Rapid Alignment Thread");
  alignment->moveToThread(thread);
  thread->start();  
  
  
  return 0
};

    for (int i = 0;i<4;i++)
        {
            alignment = new Align();
            QThread *thread = new QThread();
            thread->setObjectName("Rapid Alignment Thread");
            alignment->moveToThread(thread);
            thread->start();
            try {
                shift[i] = rapid_af::align(couple[i][0], couple[i][1], options, &ok[i]);
            }
            catch (cv::Exception e) {
                logger->warning(e.what());
            throw std::runtime_error("OpenCV exception (see log)");
            }
        }


    //
