#include "autofocuswidget.h"

#include "autofocus.h"
#include "autofocuscamdisplaywidget.h"
#include "spim.h"

#include <ICeleraCamera.h>
#include <opencv2/opencv.hpp>

#include <qtlab/widgets/cameraplot.h>

#include <QBoxLayout>
#include <QCheckBox>
#include <QDoubleSpinBox>
#include <QFileDialog>
#include <QGroupBox>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>

#define PLOT_WIDTH 1224
#define PLOT_HEIGHT 1024 


AutofocusWidget::AutofocusWidget(QWidget *parent)
    : QWidget(parent)
{
    mybufDouble1 = new double[PLOT_WIDTH * PLOT_HEIGHT];
    mybufDouble2 = new double[PLOT_WIDTH * PLOT_HEIGHT];
    setupUi();
}

AutofocusWidget::~AutofocusWidget()
{
    delete[] mybufDouble1;
    delete[] mybufDouble2;
}

void AutofocusWidget::setupUi()
{
    setEnabled(false);
    spim().getState(SPIM::STATE_READY)->assignProperty(this, "enabled", true);

    Autofocus *af = spim().getAutoFocus();
    rapid_af::AlignOptions opt = af->getOptions();

    cd1 = new AutofocusCamDisplayWidget();
    cd2 = new AutofocusCamDisplayWidget();
    cd1->setTitle(QString("Cam 1"));
    cd2->setTitle(QString("Cam 2"));
    cd1->setPlotSize(QSize(PLOT_WIDTH, PLOT_HEIGHT));
    cd2->setPlotSize(QSize(PLOT_WIDTH, PLOT_HEIGHT));
    cd1->setMinimumSize(600, 600);
    cd2->setMinimumSize(600, 600);

    connect(af, &Autofocus::newImage, this, &AutofocusWidget::onNewImage); 

    QGridLayout *grid = new QGridLayout();
    QGroupBox *generalOptionsGb = new QGroupBox("RAPID options");
    QCheckBox *multithreadingCb = new QCheckBox("Multithreading");
    QSpinBox *paddingSb = new QSpinBox();
    QDoubleSpinBox *agreementThresholdSb = new QDoubleSpinBox();
    multithreadingCb->setChecked(opt.multithreading_enable);
    paddingSb->setValue(opt.padding);
    agreementThresholdSb->setValue(opt.agreement_threshold);
    int row = 0;
    grid->addWidget(multithreadingCb, row++, 0);
    grid->addWidget(new QLabel("Padding"), row, 0);
    grid->addWidget(paddingSb, row++, 1);
    grid->addWidget(new QLabel("Agreement threshold"), row, 0);
    grid->addWidget(agreementThresholdSb, row++, 1);
    generalOptionsGb->setLayout(grid);
    paddingSb->setRange(0, 1000);

    grid = new QGridLayout();
    QGroupBox *binarizeGb = new QGroupBox("Binarize");
    binarizeGb->setCheckable(true);
    QDoubleSpinBox *binarizeThreshold = new QDoubleSpinBox();
    binarizeThreshold->setSuffix("%");
    binarizeThreshold->setDecimals(0);
    binarizeThreshold->setRange(0, 100);
    grid->addWidget(new QLabel("Threshold"), 0, 0);
    grid->addWidget(binarizeThreshold, 0, 1);
    binarizeGb->setLayout(grid);
    binarizeGb->setChecked(opt.bin_enable);
    binarizeThreshold->setValue(opt.bin_threshold * 100.);

    grid = new QGridLayout();
    QGroupBox *prefilterGb = new QGroupBox("Prefilter");
    prefilterGb->setLayout(grid);
    prefilterGb->setCheckable(true);
    row = 0;
    QSpinBox *pfKsize = new QSpinBox();
    grid->addWidget(new QLabel("kernel size"), row, 0);
    grid->addWidget(pfKsize, row++, 1);
    QDoubleSpinBox *pfSigma = new QDoubleSpinBox();
    grid->addWidget(new QLabel("σ"), row, 0);
    grid->addWidget(pfSigma, row++, 1);
    prefilterGb->setChecked(opt.prefilter_enable);
    pfKsize->setValue(opt.prefilter_ksize);
    pfSigma->setValue(opt.prefilter_sigma);

    grid = new QGridLayout();
    QGroupBox *qualityGb = new QGroupBox("Quality check");
    qualityGb->setLayout(grid);
    qualityGb->setCheckable(true);
    row = 0;
    QDoubleSpinBox *qSigma = new QDoubleSpinBox();
    QDoubleSpinBox *qSratio = new QDoubleSpinBox();
    QSpinBox *qSratioRadius = new QSpinBox();
    QSpinBox *qSratioThickness = new QSpinBox();
    grid->addWidget(new QLabel("σ threshold"), row, 0);
    grid->addWidget(qSigma, row++, 1);
    grid->addWidget(new QLabel("Spectral ratio threshold"), row, 0);
    grid->addWidget(qSratio, row++, 1);
    grid->addWidget(new QLabel("Spectral ratio radius"), row, 0);
    grid->addWidget(qSratioRadius, row++, 1);
    grid->addWidget(new QLabel("Spectral ratio thickness"), row, 0);
    grid->addWidget(qSratioThickness, row++, 1);
    qualityGb->setChecked(af->isImageQualityEnabled());
    rapid_af::ImageQualityOptions iqOpt = af->getIqOptions();
    qSratio->setDecimals(4);

    qSigma->setToolTip("minimum standard deviation");
    qSratio->setToolTip("minimum spectral ratio (within annular ring)");

    qSigma->setValue(iqOpt.sigma_threshold);
    qSratio->setValue(iqOpt.sratio_threshold);
    qSratioRadius->setValue(iqOpt.sratio_radius);
    qSratioThickness->setValue(iqOpt.sratio_thickness);

    grid = new QGridLayout();
    QGroupBox *dogGb = new QGroupBox("DoG");
    dogGb->setCheckable(true);
    row = 0;
    QSpinBox *dogKsize = new QSpinBox();
    grid->addWidget(new QLabel("kernel size"), row, 0);
    grid->addWidget(dogKsize, row++, 1);
    QDoubleSpinBox *dogSigma1 = new QDoubleSpinBox();
    grid->addWidget(new QLabel("σ1"), row, 0);
    grid->addWidget(dogSigma1, row++, 1);
    QDoubleSpinBox *dogSigma2 = new QDoubleSpinBox();
    grid->addWidget(new QLabel("σ2"), row, 0);
    grid->addWidget(dogSigma2, row++, 1);
    dogGb->setLayout(grid);
    dogKsize->setRange(0, 1000);
    dogSigma1->setRange(0, 1000);
    dogSigma2->setRange(0, 1000);
    dogGb->setChecked(opt.dog_enable);
    dogKsize->setValue(opt.dog_ksize);
    dogSigma1->setValue(opt.dog_sigma1);
    dogSigma2->setValue(opt.dog_sigma2);

    grid = new QGridLayout();
    QGroupBox *cannyGb = new QGroupBox("Canny filter");
    cannyGb->setCheckable(true);
    row = 0;
    QSpinBox *cannyKsize = new QSpinBox();
    grid->addWidget(new QLabel("kernel size"), row, 0);
    grid->addWidget(cannyKsize, row++, 1);
    QDoubleSpinBox *cannySigma = new QDoubleSpinBox();
    grid->addWidget(new QLabel("σ"), row, 0);
    grid->addWidget(cannySigma, row++, 1);
    QDoubleSpinBox *cannyAlpha = new QDoubleSpinBox();
    grid->addWidget(new QLabel("α"), row, 0);
    grid->addWidget(cannyAlpha, row++, 1);
    QDoubleSpinBox *cannyBeta = new QDoubleSpinBox();
    grid->addWidget(new QLabel("β"), row, 0);
    grid->addWidget(cannyBeta, row++, 1);
    cannyGb->setLayout(grid);
    cannyKsize->setRange(0, 1000);
    cannySigma->setRange(0, 1000);
    cannyAlpha->setRange(0, 1000);
    cannyBeta->setRange(0, 1000);
    cannyGb->setChecked(opt.canny_enable);
    cannyKsize->setValue(opt.canny_ksize);
    cannySigma->setValue(opt.canny_sigma);
    cannyAlpha->setValue(opt.canny_alpha);
    cannyBeta->setValue(opt.canny_beta);
    cannyAlpha->setToolTip("Lower threshold for hysteresis thresholding");
    cannyBeta->setToolTip("Higher threshold for hysteresis thresholding");
    cannySigma->setToolTip("Smoothing sigma for Canny filter");
    cannyKsize->setToolTip("Kernel size for Canny filter");

    QSpinBox *exposureTimeSb = new QSpinBox();
    exposureTimeSb->setRange(28, 5e6);
    exposureTimeSb->setSuffix("µs");
    exposureTimeSb->setValue(af->getExposureTime_us());

    QSpinBox *frameRateSb = new QSpinBox();
    frameRateSb->setRange(1, 25);
    frameRateSb->setSuffix("Hz");
    frameRateSb->setValue(af->getFrameRate());

    grid = new QGridLayout();
    QGroupBox *cameraGb = new QGroupBox("Camera");
    cameraGb->setLayout(grid);
    row = 0;
    grid->addWidget(new QLabel("Exposure"), row, 0);
    grid->addWidget(exposureTimeSb, row++, 1);
    grid->addWidget(new QLabel("Frame rate"), row, 0);
    grid->addWidget(frameRateSb, row++, 1);

    QDoubleSpinBox *mSbAlpha = new QDoubleSpinBox();
    QDoubleSpinBox *qSbAlpha = new QDoubleSpinBox();  
    QDoubleSpinBox *mSbBeta1 = new QDoubleSpinBox();
    QDoubleSpinBox *qSbBeta1 = new QDoubleSpinBox();   
    QDoubleSpinBox *mSbBeta2 = new QDoubleSpinBox();
    QDoubleSpinBox *qSbBeta2 = new QDoubleSpinBox();

    mSbAlpha->setRange(-1e10, 1e10);
    qSbAlpha->setRange(-1e10, 1e10);
    mSbBeta1->setRange(-1e10, 1e10);
    qSbBeta1->setRange(-1e10, 1e10);
    mSbBeta2->setRange(-1e10, 1e10);
    qSbBeta2->setRange(-1e10, 1e10);
    
    mSbAlpha->setDecimals(6);
    qSbAlpha->setDecimals(6);
    mSbBeta1->setDecimals(6);
    qSbBeta1->setDecimals(6);
    mSbBeta2->setDecimals(6);
    qSbBeta2->setDecimals(6);
    
    mSbAlpha->setValue(af->getCalibration_mAlpha());
    qSbAlpha->setValue(af->getCalibration_qAlpha());
    mSbBeta1->setValue(af->getCalibration_mBeta1());
    qSbBeta1->setValue(af->getCalibration_qBeta1());
    mSbBeta2->setValue(af->getCalibration_mBeta2());
    qSbBeta2->setValue(af->getCalibration_qBeta2());

    grid = new QGridLayout();
    QGroupBox *calibrationGbAlpha = new QGroupBox("Alpha Calibration");
    calibrationGbAlpha->setLayout(grid);
    QPushButton *setQPbAlpha = new QPushButton("Set Q from correction");
    row = 0;
    grid->addWidget(new QLabel("m alpha"), row, 0);
    grid->addWidget(mSbAlpha, row++, 1);
    grid->addWidget(new QLabel("q alpha"), row, 0);
    grid->addWidget(qSbAlpha, row++, 1);
    grid->addWidget(setQPbAlpha, row++, 0, 1, 2);
    connect(setQPbAlpha, &QPushButton::clicked, [=]() {
        try {
            qSbAlpha->setValue(af->inferCalibrationQAlpha());
        } catch (std::runtime_error e) {
            QMessageBox::critical(this, "Error", e.what());
        }
    });

    grid = new QGridLayout();
    QGroupBox *calibrationGbBeta1 = new QGroupBox("Beta 1 Calibration");
    calibrationGbBeta1->setLayout(grid);
    QPushButton *setQPbBeta1 = new QPushButton("Set Q from correction");
    row = 0;
    grid->addWidget(new QLabel("m beta 1"), row, 0);
    grid->addWidget(mSbBeta1, row++, 1);
    grid->addWidget(new QLabel("q beta 1"), row, 0);
    grid->addWidget(qSbBeta1, row++, 1);
    grid->addWidget(setQPbBeta1, row++, 0, 1, 2);
    connect(setQPbBeta1, &QPushButton::clicked, [=]() {
        try {
            qSbBeta1->setValue(af->inferCalibrationQBeta1());
        } catch (std::runtime_error e) {
            QMessageBox::critical(this, "Error", e.what());
        }
    });

    grid = new QGridLayout();
    QGroupBox *calibrationGbBeta2 = new QGroupBox("Beta 2 Calibration");
    calibrationGbBeta2->setLayout(grid);
    QPushButton *setQPbBeta2 = new QPushButton("Set Q from correction");
    row = 0;
    grid->addWidget(new QLabel("m beta 2"), row, 0);
    grid->addWidget(mSbBeta2, row++, 1);
    grid->addWidget(new QLabel("q beta 2"), row, 0);
    grid->addWidget(qSbBeta2, row++, 1);
    grid->addWidget(setQPbBeta2, row++, 0, 1, 2);
    connect(setQPbBeta2, &QPushButton::clicked, [=]() {
        try {
            qSbBeta2->setValue(af->inferCalibrationQBeta2());
        } catch (std::runtime_error e) {
            QMessageBox::critical(this, "Error", e.what());
        }
    });
    
    grid = new QGridLayout();
    QGroupBox *autofocusGb = new QGroupBox("Autofocus");
    autofocusGb->setLayout(grid);
    row = 0;
    QCheckBox *enableCb = new QCheckBox("Enable");
    QCheckBox *outputEnableCb = new QCheckBox("Output enable");
    enableCb->setChecked(af->isEnabled());
    outputEnableCb->setChecked(af->isOutputEnabled());
    connect(enableCb, &QCheckBox::toggled, [=](bool checked) {
        outputEnableCb->setEnabled(checked);
    });
    QLabel *statusLabel = new QLabel();
    grid->addWidget(enableCb, row++, 0);
    grid->addWidget(outputEnableCb, row++, 0);
    grid->addWidget(new QLabel("Status: "), row++, 0);
    grid->addWidget(statusLabel, row++, 0);
    autofocusGb->setLayout(grid);
    
    pmw = new PixmapWidget[4];
    QList<QImage> mergedList;
    try{
        mergedList=af->getMergedImage(); //here we have a list with 4 images
    } catch (std::runtime_error e) {
        emit af->newStatus(e.what());
        return;
    }   
    
    connect(af, &Autofocus::newStatus, [=](QString s) {
        statusLabel->setText(s);
        if (isVisible() && s.startsWith("dx")) 
        {
            for (int i=0,i<3,i++)
                {
                    pmw[i]->setPixmap(QPixmap::fromImage(mergedList[i])); //we convert them to QPixmap
                } 
        }
    });

    QPushButton *saveToFile = new QPushButton("Save image");

    connect(saveToFile, &QPushButton::clicked, [=]() {
        QString fileName = QFileDialog::getSaveFileName(this, tr("Save File"), "autofocusalpha.tiff");

        if (fileName.isEmpty()) {
            return;
        }

        cv::Mat img(ptr.GetHeight(), ptr.GetWidth(), CV_16U, (void *) ptr.Data());

        try {
            cv::imwrite(fileName.toStdString(), img);
        } catch (cv::Exception e) {
            QMessageBox::critical(this, "Error", e.what());
        }
    });

    QPushButton *saveImage1ToFile = new QPushButton("Save left ROI");

    connect(saveImage1File, &QPushButton::clicked, [=]() {
        QString fileName = QFileDialog::getSaveFileName(this, tr("Save File"), "left_ROI.tiff");

        if (fileName.isEmpty()) {
            return;
        }

        try {
            cv::imwrite(fileName.toStdString(), af->getImage1());
        } catch (cv::Exception e) {
            QMessageBox::critical(this, "Error", e.what());
        }
    });

    QPushButton *saveImage2ToFile = new QPushButton("Save right ROI");

    connect(saveImage2ToFile, &QPushButton::clicked, [=]() {
        QString fileName = QFileDialog::getSaveFileName(this, tr("Save File"), "right_ROI.tiff");

        if (fileName.isEmpty()) {
            return;
        }

        try {
            cv::imwrite(fileName.toStdString(), af->getImage2());
        } catch (cv::Exception e) {
            QMessageBox::critical(this, "Error", e.what());
        }
    });

    auto applyOptions = [=]() {
        rapid_af::AlignOptions opt;

        opt.multithreading_enable = multithreadingCb->isChecked();
        opt.padding = paddingSb->value();
        opt.agreement_threshold = agreementThresholdSb->value();

        opt.prefilter_enable = prefilterGb->isChecked();
        opt.prefilter_ksize = pfKsize->value();
        opt.prefilter_sigma = pfSigma->value();

        opt.bin_enable = binarizeGb->isChecked();
        opt.bin_threshold = binarizeThreshold->value() / 100.;

        opt.dog_enable = dogGb->isChecked();
        opt.dog_ksize = dogKsize->value();
        opt.dog_sigma1 = dogSigma1->value();
        opt.dog_sigma2 = dogSigma2->value();

        opt.canny_enable = cannyGb->isChecked();
        opt.canny_ksize = cannyKsize->value();
        opt.canny_sigma = cannySigma->value();
        opt.canny_alpha = cannyAlpha->value();
        opt.canny_beta = cannyBeta->value();

        af->setOptions(opt);

        rapid_af::ImageQualityOptions iqOpt;
        iqOpt.sigma_threshold = qSigma->value();
        iqOpt.sratio_threshold = qSratio->value();
        iqOpt.sratio_radius = qSratioRadius->value();
        iqOpt.sratio_thickness = qSratioThickness->value();

        af->setImageQualityEnabled(qualityGb->isChecked());
        af->setIqOptions(iqOpt);

        af->setCalibration(mSb->value(), qSb->value());         //tf did i do?
        af->setOutputEnabled(outputEnableCb->isChecked());
    };

    auto applyOptionsAndRestart = [=]() {
        af->setExposureTime_us(exposureTimeSb->value());
        af->setFrameRate(frameRateSb->value());

        af->setEnabled(enableCb->isChecked());

        spim().restartAutofocus();
    };

    QList<QGroupBox *> gbList = {qualityGb, prefilterGb, binarizeGb, dogGb, cannyGb};
    for (auto gb : gbList) {
        connect(gb, &QGroupBox::toggled, [=]() { applyOptions(); });
    }

    QList<QCheckBox *> cbList = {multithreadingCb, outputEnableCb};
    for (auto cb : cbList) {
        connect(cb, &QCheckBox::toggled, [=]() { applyOptions(); });
    }

    connect(enableCb, &QCheckBox::toggled, [=]() { applyOptionsAndRestart(); });

    QList<QSpinBox *> sbList = {
        paddingSb,
        pfKsize,
        qSratioRadius,
        qSratioThickness,
        dogKsize,
        cannyKsize,

    };
    for (auto sb : sbList) {
        connect(sb, QOverload<int>::of(&QSpinBox::valueChanged), [=]() { applyOptions(); });
    }

    sbList = {exposureTimeSb, frameRateSb};
    for (auto sb : sbList) {
        connect(sb, QOverload<int>::of(&QSpinBox::valueChanged), [=]() {
            applyOptionsAndRestart();
        });
    }

    QList<QDoubleSpinBox *> dsbList = {
        agreementThresholdSb,
        binarizeThreshold,
        qSigma,
        qSratio,
        qSb,
        mSb,
        pfSigma,
        dogSigma1,
        dogSigma2,
        cannySigma,
        cannyAlpha,
        cannyBeta,
    };
    for (auto dsb : dsbList) {
        connect(dsb, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [=]() {
            applyOptions();
        });
    }

    QBoxLayout *vLayout1 = new QVBoxLayout();
    vLayout1->addWidget(cd1);
    vLayout1->addStretch();
    vLayout1->addWidget(cd2);
    
    QBoxLayout *vLayout2 = new QVBoxLayout();
    QBoxLayout *hLayout1 = new QHBoxLayout();
    hLayout1->addStretch();
    hLayout1->addWidget(pmw[0]);
    hLayout1->addStretch();
    hLayout1->addWidget(pmw[1]);
    hLayout1->addStretch();
    QBoxLayout *hLayout2 = new QHBoxLayout();
    hLayout2->addStretch();
    hLayout2->addWidget(pmw[2]);
    hLayout2->addStretch();
    hLayout2->addWidget(pmw[3]);
    hLayout2->addStretch();
    vLayout2->addLayout(hLayout1);
    vLayout2->addLayout(hLayout2);
    
    QBoxLayout *vLayout3 = new QVBoxLayout();
    vLayout3->addWidget(prefilterGb);
    vLayout3->addWidget(qualityGb);
    vLayout3->addWidget(binarizeGb);
    vLayout3->addWidget(dogGb);
    vLayout3->addWidget(cannyGb);
    vLayout3->addStretch();
    vLayout3->addStretch();

    QBoxLayout *vLayout4 = new QVBoxLayout();
    vLayout4->addWidget(cameraGb);
    vLayout4->addWidget(calibrationGbAlpha);
    vLayout4->addWidget(calibrationGbBeta1);
    vLayout4->addWidget(calibrationGbBeta2);
    vLayout4->addWidget(generalOptionsGb);
    vLayout4->addStretch();
    vLayout4->addWidget(autofocusGb);
    vLayout4->addWidget(saveLeftRoiToFile);
    vLayout4->addWidget(saveRightRoiToFile);
    vLayout4->addWidget(saveToFile);

    hLayout = new QHBoxLayout();
    hLayout->addLayout(vLayout1);
    hLayout->addLayout(vLayout2);
    hLayout->addLayout(vLayout3);
    hLayout->addLayout(vLayout4);
    setLayout(hLayout);
}

void AutofocusWidget::onNewImage(QList<CAlkUSB3::BufferPtr> ptr)
{
    this->ptr[0] = ptr[0];
    if (!isVisible()) {
        return;
    }
    this->ptr[1] = ptr[1];
    if (!isVisible()) {
        return;
    }

    cd1->setPlotSize(QSize(ptr[0].GetWidth(), ptr[0].GetHeight()));
    cd2->setPlotSize(QSize(ptr[1].GetWidth(), ptr[1].GetHeight()));
    
    quint8 *buf1 = (quint8 *) (ptr[0].Data());
    int c = 0;
    for (size_t i = 0; i < ptr[0].GetWidth(); i += 1) {
        for (size_t j = 0; j < ptr[0].GetHeight(); j += 1) {
            mybufDouble1[c] = buf1[c];
            c++;
        }
    }
    cd1->getPlot()->setData(mybufDouble1, c);    
    
    quint8 *buf2 = (quint8 *) (ptr[1].Data());
    int c = 0;
    for (size_t i = 0; i < ptr[1].GetWidth(); i += 1) {
        for (size_t j = 0; j < ptr[1].GetHeight(); j += 1) {
            mybufDouble2[c] = buf2[c];
            c++;
        }
    }
    cd2->getPlot()->setData(mybufDouble2, c);
}
