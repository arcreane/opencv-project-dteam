#include "MainWindow.h"
#include "ClickableLabel.h"
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QToolBar>
#include <QFileDialog>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QGroupBox>
#include <QPushButton>
#include <QMouseEvent>
#include <QInputDialog>
#include <QDialog>
#include <QListWidget>
#include <QDialogButtonBox>
#include <QScrollArea>
#include <QApplication>
#include <QScreen>
#include <QCheckBox>
#include <QFileInfo>
#include <opencv2/imgcodecs.hpp>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent), m_undo(20)
{
    setWindowTitle("MyEditor");
    resize(1280, 800);

    setupCentralWidget();
    setupDockPanels();
    setupMenuBar();
    setupToolBar();

    statusBar()->showMessage("Ouvrez une image pour commencer.");
}
void MainWindow::setupCentralWidget() {
    auto* label = new ClickableLabel(this);
    label->setAlignment(Qt::AlignCenter);
    label->setStyleSheet("background-color: #2b2b2b;");
    label->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    m_imageLabel = label;
    label->onClickHandler = [this](QPoint p){ onImageClicked(p); };
    label->onMoveHandler  = [this](QPoint p){ updateStatusBar(p); };
    m_scrollArea = new QScrollArea(this);
    m_scrollArea->setWidget(m_imageLabel);
    m_scrollArea->setWidgetResizable(false);
    m_scrollArea->setAlignment(Qt::AlignCenter);
    setCentralWidget(m_scrollArea);
}
void MainWindow::setupDockPanels() {
    m_paramDock = new QDockWidget("Paramètres", this);
    m_paramDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    auto* container = new QWidget;
    auto* vbox = new QVBoxLayout(container);
    vbox->setSpacing(8);
    {
        auto* grp = new QGroupBox("Seuillage");
        auto* lay = new QVBoxLayout(grp);
        m_threshMode = new QComboBox;
        m_threshMode->addItems({"Binary", "Otsu", "Adaptatif"});
        m_threshSlider = new QSlider(Qt::Horizontal);
        m_threshSlider->setRange(0, 255);
        m_threshSlider->setValue(128);
        lay->addWidget(new QLabel("Mode :"));
        lay->addWidget(m_threshMode);
        lay->addWidget(new QLabel("Seuil :"));
        lay->addWidget(m_threshSlider);
        auto* btn = new QPushButton("Appliquer");
        connect(btn, &QPushButton::clicked, this, &MainWindow::applyThreshold);
        lay->addWidget(btn);
        vbox->addWidget(grp);
    }
    {
        auto* grp = new QGroupBox("Morphologie");
        auto* lay = new QVBoxLayout(grp);
        m_morphOp = new QComboBox;
        m_morphOp->addItems({"Dilatation","Érosion","Opening","Closing","Gradient"});
        m_morphKernel = new QSlider(Qt::Horizontal);
        m_morphKernel->setRange(1, 21);
        m_morphKernel->setSingleStep(2);
        m_morphKernel->setValue(3);
        lay->addWidget(new QLabel("Opération :"));
        lay->addWidget(m_morphOp);
        lay->addWidget(new QLabel("Taille noyau :"));
        lay->addWidget(m_morphKernel);
        auto* btn = new QPushButton("Appliquer");
        connect(btn, &QPushButton::clicked, this, &MainWindow::applyMorphology);
        lay->addWidget(btn);
        vbox->addWidget(grp);
    }
    {
        auto* grp = new QGroupBox("Canny");
        auto* lay = new QVBoxLayout(grp);
        m_cannyLow  = new QSlider(Qt::Horizontal); m_cannyLow->setRange(0,255);  m_cannyLow->setValue(50);
        m_cannyHigh = new QSlider(Qt::Horizontal); m_cannyHigh->setRange(0,255); m_cannyHigh->setValue(150);
        lay->addWidget(new QLabel("Seuil bas :")); lay->addWidget(m_cannyLow);
        lay->addWidget(new QLabel("Seuil haut :")); lay->addWidget(m_cannyHigh);
        auto* btn = new QPushButton("Appliquer");
        connect(btn, &QPushButton::clicked, this, &MainWindow::applyCanny);
        lay->addWidget(btn);
        vbox->addWidget(grp);
    }
    {
        auto* grp = new QGroupBox("Correction Gamma");
        auto* lay = new QVBoxLayout(grp);
        m_gammaSlider = new QSlider(Qt::Horizontal);
        m_gammaSlider->setRange(10, 300);
        m_gammaSlider->setValue(100);
        lay->addWidget(new QLabel("γ (×100) :"));
        lay->addWidget(m_gammaSlider);
        auto* btn = new QPushButton("Appliquer");
        connect(btn, &QPushButton::clicked, this, &MainWindow::applyGammaCorrection);
        lay->addWidget(btn);
        vbox->addWidget(grp);
    }
    vbox->addStretch();
    m_paramDock->setWidget(container);
    addDockWidget(Qt::LeftDockWidgetArea, m_paramDock);
    auto* histDock = new QDockWidget("Histogramme", this);
    m_histogram = new HistogramWidget;
    histDock->setWidget(m_histogram);
    addDockWidget(Qt::RightDockWidgetArea, histDock);
    m_modeLabel = new QLabel("");
    statusBar()->addPermanentWidget(m_modeLabel);
}
void MainWindow::setupMenuBar() {
    auto* fileMenu = menuBar()->addMenu("Fichier");
    fileMenu->addAction("Ouvrir...",    this, &MainWindow::openImage,  QKeySequence::Open);
    fileMenu->addAction("Enregistrer",this, &MainWindow::saveImage,  QKeySequence::Save);
    fileMenu->addAction("Enregistrer sous...", this, &MainWindow::saveImageAs);
    fileMenu->addSeparator();
    fileMenu->addAction("Quitter", qApp, &QApplication::quit, QKeySequence::Quit);
    auto* editMenu = menuBar()->addMenu("Edition");
    editMenu->addAction("Annuler",  this, &MainWindow::undo, QKeySequence::Undo);
    editMenu->addAction("Retablir", this, &MainWindow::redo, QKeySequence::Redo);
    auto* coreMenu = menuBar()->addMenu("Filtres de base");
    coreMenu->addAction("Seuillage",               this, &MainWindow::applyThreshold);
    coreMenu->addAction("Egalisation histogramme", this, &MainWindow::applyHistogramEq);
    coreMenu->addAction("Morphologie",             this, &MainWindow::applyMorphology);
    coreMenu->addAction("Detection contours Canny",this, &MainWindow::applyCanny);
    coreMenu->addAction("Transformation geometrique", this, &MainWindow::applyGeometric);
    coreMenu->addAction("Panorama / Stitching",    this, &MainWindow::openPanoramaDialog);
    auto* advMenu = menuBar()->addMenu("Avance");
    advMenu->addAction("Effet Cartoon",    this, &MainWindow::applyCartoon);
    advMenu->addAction("Esquisse crayon",  this, &MainWindow::applyPencilSketch);
    advMenu->addAction("Filtre bilateral", this, &MainWindow::applyBilateral);
    advMenu->addAction("Detection ORB",    this, &MainWindow::applyORBDetection);
    advMenu->addAction("Vignette + grain", this, &MainWindow::applyVignetteGrain);
    advMenu->addAction("Baguette magique", this, &MainWindow::activateFloodFill);
    auto* viewMenu = menuBar()->addMenu("Vue");
    viewMenu->addAction("Zoom +",  this, &MainWindow::zoomIn,      QKeySequence(Qt::CTRL | Qt::Key_Plus));
    viewMenu->addAction("Zoom -",  this, &MainWindow::zoomOut,     QKeySequence(Qt::CTRL | Qt::Key_Minus));
    viewMenu->addAction("Ajuster", this, &MainWindow::fitToWindow,  QKeySequence(Qt::CTRL | Qt::Key_0));
}
void MainWindow::setupToolBar() {
    auto* tb = addToolBar("Principal");
    tb->addAction("Ouvrir",   this, &MainWindow::openImage);
    tb->addAction("Sauver",   this, &MainWindow::saveImage);
    tb->addSeparator();
    tb->addAction("Annuler",  this, &MainWindow::undo);
    tb->addAction("Retablir", this, &MainWindow::redo);
    tb->addSeparator();
    tb->addAction("Zoom+",    this, &MainWindow::zoomIn);
    tb->addAction("Zoom-",    this, &MainWindow::zoomOut);
    tb->addAction("Ajuster",  this, &MainWindow::fitToWindow);
}
void MainWindow::pushUndo(const std::string& desc) {
    m_undo.push(m_current, desc);
}
void MainWindow::setCurrentImage(const cv::Mat& img, const std::string& undoDesc) {
    pushUndo(undoDesc);
    m_current = img.clone();
    updateDisplay();
}
void MainWindow::updateDisplay() {
    if (m_current.empty()) return;
    cv::Mat display = m_current.clone();
    cv::Mat rgb;
    if (display.channels() == 3)
        cv::cvtColor(display, rgb, cv::COLOR_BGR2RGB);
    else if (display.channels() == 1)
        cv::cvtColor(display, rgb, cv::COLOR_GRAY2RGB);
    else
        rgb = display;
    QImage qimg(rgb.data, rgb.cols, rgb.rows, static_cast<int>(rgb.step),
                QImage::Format_RGB888);
    QPixmap pix = QPixmap::fromImage(qimg);
    if (m_zoomFactor != 1.0) {
        pix = pix.scaled(
            static_cast<int>(pix.width()  * m_zoomFactor),
            static_cast<int>(pix.height() * m_zoomFactor),
            Qt::KeepAspectRatio, Qt::SmoothTransformation
        );
    }
    m_imageLabel->setPixmap(pix);
    m_imageLabel->adjustSize();
    m_histogram->setImage(m_current);
    updateStatusBar();
}
cv::Mat MainWindow::getPreviewImage() const {
    return ImageProcessor::fitToSize(m_current, 800, 600);
}
void MainWindow::updateStatusBar(QPoint pos) {
    if (m_current.empty()) return;
    QString msg = QString("%1 x %2 px  |  Zoom %3%")
        .arg(m_current.cols).arg(m_current.rows)
        .arg(static_cast<int>(m_zoomFactor * 100));
    if (pos.x() >= 0)
        msg += QString("  |  curseur (%1, %2)").arg(pos.x()).arg(pos.y());
    statusBar()->showMessage(msg);
}
void MainWindow::openImage() {
    QString path = QFileDialog::getOpenFileName(this, "Ouvrir une image", "",
        "Images (*.png *.jpg *.jpeg *.bmp *.tiff *.webp);;Tous (*.*)");
    if (path.isEmpty()) return;
    cv::Mat img = cv::imread(path.toStdString());
    if (img.empty()) {
        QMessageBox::critical(this, "Erreur", "Impossible de lire l'image.");
        return;
    }
    m_filePath = path.toStdString();
    m_original = img.clone();
    m_current  = img.clone();
    m_undo.clear();
    m_undo.push(m_current, "Ouverture");
    m_zoomFactor = 1.0;
    updateDisplay();
    setWindowTitle("MyEditor — " + QFileInfo(path).fileName());
}
void MainWindow::saveImage() {
    if (m_current.empty()) return;
    if (m_filePath.empty()) { saveImageAs(); return; }
    cv::imwrite(m_filePath, m_current);
    statusBar()->showMessage("Image enregistree : " + QString::fromStdString(m_filePath));
}
void MainWindow::saveImageAs() {
    if (m_current.empty()) return;
    QString path = QFileDialog::getSaveFileName(this, "Enregistrer sous", "",
        "PNG (*.png);;JPEG (*.jpg);;BMP (*.bmp)");
    if (path.isEmpty()) return;
    m_filePath = path.toStdString();
    cv::imwrite(m_filePath, m_current);
    statusBar()->showMessage("Enregistre : " + path);
}
void MainWindow::undo() {
    cv::Mat prev;
    if (m_undo.undo(prev)) {
        m_current = prev;
        updateDisplay();
        statusBar()->showMessage("Annule : " + QString::fromStdString(m_undo.lastDescription()));
    }
}
void MainWindow::redo() {
    cv::Mat next;
    if (m_undo.redo(next)) {
        m_current = next;
        updateDisplay();
    }
}
void MainWindow::applyThreshold() {
    if (m_current.empty()) return;
    int mode   = m_threshMode->currentIndex();
    int thresh = m_threshSlider->value();
    cv::Mat result = ImageProcessor::threshold(m_current, mode, thresh);
    setCurrentImage(result, "Seuillage");
}
void MainWindow::applyHistogramEq() {
    if (m_current.empty()) return;
    bool useCLAHE = QMessageBox::question(this, "Egalisation",
        "Utiliser CLAHE (adaptatif) ?") == QMessageBox::Yes;
    cv::Mat result = ImageProcessor::equalizeHistogram(m_current, useCLAHE);
    setCurrentImage(result, "Egalisation histogramme");
}
void MainWindow::applyMorphology() {
    if (m_current.empty()) return;
    int op   = m_morphOp->currentIndex();
    int size = m_morphKernel->value();
    if (size % 2 == 0) size++;
    cv::Mat result = ImageProcessor::morphology(m_current, op, size);
    setCurrentImage(result, "Morphologie");
}
void MainWindow::applyCanny() {
    if (m_current.empty()) return;
    int lo = m_cannyLow->value();
    int hi = m_cannyHigh->value();
    cv::Mat result = ImageProcessor::canny(m_current, lo, hi);
    setCurrentImage(result, "Canny");
}
void MainWindow::applyGeometric() {
    if (m_current.empty()) return;
    // Transformation perspective prédéfinie (inclinaison droite légère)
    int w = m_current.cols, h = m_current.rows;
    std::vector<cv::Point2f> src = {
        {0.f,0.f},{(float)w,0.f},{(float)w,(float)h},{0.f,(float)h}
    };
    std::vector<cv::Point2f> dst = {
        {0.f,0.f},{(float)w*0.9f,(float)h*0.05f},
        {(float)w*0.9f,(float)h*0.95f},{0.f,(float)h}
    };
    cv::Mat result = ImageProcessor::geometricTransform(m_current, 1, src, dst);
    setCurrentImage(result, "Transformation geometrique");
}
void MainWindow::openPanoramaDialog() {
    QStringList files = QFileDialog::getOpenFileNames(this,
        "Selectionnez les images a assembler (ordre gauche > droite)", "",
        "Images (*.png *.jpg *.jpeg *.bmp)");
    if (files.size() < 2) {
        QMessageBox::information(this, "Panorama",
            "Selectionnez au moins 2 images pour creer un panorama.");
        return;
    }
    std::vector<cv::Mat> imgs;
    for (const auto& f : files) {
        cv::Mat img = cv::imread(f.toStdString());
        if (!img.empty()) imgs.push_back(img);
    }
    cv::Mat pano;
    if (m_pano.stitch(imgs, pano)) {
        setCurrentImage(pano, "Panorama");
        statusBar()->showMessage("Panorama cree avec succes !");
    } else {
        QMessageBox::warning(this, "Panorama",
            "Echec : " + QString::fromStdString(m_pano.lastError()));
    }
}
void MainWindow::applyCartoon() {
    if (m_current.empty()) return;
    setCurrentImage(ImageProcessor::cartoonEffect(m_current), "Cartoon");
}
void MainWindow::applyPencilSketch() {
    if (m_current.empty()) return;
    setCurrentImage(ImageProcessor::pencilSketch(m_current), "Pencil Sketch");
}
void MainWindow::applyBilateral() {
    if (m_current.empty()) return;
    setCurrentImage(ImageProcessor::bilateralFilter(m_current), "Bilateral");
}
void MainWindow::applyORBDetection() {
    if (m_current.empty()) return;
    setCurrentImage(ImageProcessor::detectORB(m_current), "ORB Detection");
}
void MainWindow::applyGammaCorrection() {
    if (m_current.empty()) return;
    double gamma = m_gammaSlider->value() / 100.0;
    setCurrentImage(ImageProcessor::gammaCorrection(m_current, gamma), "Gamma");
}
void MainWindow::applyVignetteGrain() {
    if (m_current.empty()) return;
    setCurrentImage(ImageProcessor::vignetteGrain(m_current, 0.5, 12), "Vignette+Grain");
}
void MainWindow::activateFloodFill() {
    m_floodFillMode = !m_floodFillMode;
    m_modeLabel->setText(m_floodFillMode ? "Baguette magique active (cliquez sur l'image)" : "");
    statusBar()->showMessage(m_floodFillMode ? "Cliquez sur une zone pour la remplir." : "");
}
void MainWindow::onImageClicked(QPoint pos) {
    if (!m_floodFillMode || m_current.empty()) return;
    double scaleX = static_cast<double>(m_current.cols) / m_imageLabel->width();
    double scaleY = static_cast<double>(m_current.rows) / m_imageLabel->height();
    cv::Point seed(static_cast<int>(pos.x() * scaleX),
                   static_cast<int>(pos.y() * scaleY));
    cv::Mat result = ImageProcessor::floodFillTool(m_current, seed,
                                                    cv::Scalar(0, 0, 255), 30);
    setCurrentImage(result, "Flood Fill");
    m_floodFillMode = false;
    m_modeLabel->setText("");
}
void MainWindow::zoomIn() {
    m_zoomFactor = std::min(m_zoomFactor * 1.25, 8.0);
    updateDisplay();
}
void MainWindow::zoomOut() {
    m_zoomFactor = std::max(m_zoomFactor / 1.25, 0.1);
    updateDisplay();
}
void MainWindow::fitToWindow() {
    m_zoomFactor = 1.0;
    updateDisplay();
}
