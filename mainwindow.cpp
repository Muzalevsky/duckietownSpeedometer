#include "mainwindow.h"


#include <QDebug>
#include <QFileInfo>

using namespace cv;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
}

Point MainWindow::getPointFromImage(QString img_path)
{
    Mat im_src, im_gray, im_bin, im_blur, im_dst, im_color_dst, im_dilate;

    qDebug() << img_path;
    im_src = imread(img_path.toStdString(), 1);
    if (im_src.empty())
        return Point();

    qDebug() << im_src.size.dims();
    qDebug() << im_src.cols;
    qDebug() << im_src.rows;
    namedWindow( "Loaded", 1 );
    imshow( "Loaded", im_src );

    //ROI
    //2) Tranform
    //    "src_bbox": [[0, 240], [320, 240], [320, 480], [0, 480]]}
    QVector<Point2f> roi;
    roi << Point2f( 0, 240 ) << Point2f( 200, 240 ) << Point2f( 200, 480 ) << Point2f( 0, 480 );

    QVector<Point2f> output_roi;
    output_roi << Point2f( 0, 0 ) << Point2f( 320, 0 ) << Point2f( 320, 480 ) << Point2f( 0, 480 );

    Mat transform = getPerspectiveTransform( roi.data(), output_roi.data() );

    Mat im_transf;
    warpPerspective( im_src, im_transf, transform, cv::Size(500,500) );
    namedWindow( "Perspective", 1 );
    imshow( "Perspective", im_transf );

    // preparing the mask to overlay
    Mat mask;
    // RGB?
    // BGR!
    inRange(im_transf, Scalar(100, 200, 200), Scalar(200, 235, 225), mask);
    namedWindow( "Colored", 1 );
    imshow( "Colored", mask );

    //X) Get rid of small details
//    int filter_sz = 2;
//    GaussianBlur(im_transf, im_blur, cv::Size(filter_sz, filter_sz), 0);
    int blur_size = 2;
    blur( mask, im_blur, Size(blur_size, blur_size) );
    namedWindow( "Blur", 1 );
    imshow( "blur", im_blur );

    //Morph
    Mat im_morph;
    Mat kernel;
    kernel.zeros( 5, 5, CV_8U );
    morphologyEx(im_blur, im_morph, MORPH_CLOSE, kernel);
    namedWindow( "Morph", 1 );
    imshow( "Morph", im_morph );

    //4) Dilate
    int dilation_size = 2;
    Mat element = getStructuringElement( MORPH_ELLIPSE,
                           Size( 2*dilation_size + 1, 2*dilation_size+1 ),
                           Point( dilation_size, dilation_size ) );
    dilate(im_morph, im_dilate, element);
    namedWindow( "Dilation", 1 );
    imshow( "Dilation", im_dilate );

    Mat im_erode;
    Mat kernel2;
    kernel2.zeros( 3, 3, CV_8U );

    erode(im_dilate, im_erode, kernel2 );
    namedWindow( "im_erode", 1 );
    imshow( "im_erode", im_erode );


    //3) Binarize
    threshold(im_erode, im_bin, 100.0, 255.0, THRESH_BINARY | THRESH_OTSU);
    namedWindow( "Binarize", 1 );
    imshow( "Binarize", im_bin );

    Canny( im_bin, im_dst, 50, 200, 3 );
    imshow( "im_dst", im_dst );

    std::vector<std::vector<Point> > contours;
    std::vector<Vec4i> hierarchy;
    findContours(im_dst, contours, RETR_LIST, CHAIN_APPROX_TC89_L1);
    std::vector<double> areas;

    for( size_t i = 0; i< contours.size(); i++ )
    {
        areas.push_back( contourArea( contours[i] ) );
        if ( areas[i] > 200 ) {
            drawContours( im_transf, contours, (int)i, Scalar(128), 2, LINE_8, hierarchy, 0 );
        }
    }


    std::vector<std::vector<Point> > bigContours;
    int bigCountourCount = 4;
    for ( int i = 0; i < bigCountourCount; i++ )
    {
        int maxElementIndex = std::distance(areas.begin(),std::max_element(areas.begin(), areas.end()));
        qDebug() << maxElementIndex << areas[maxElementIndex];
        areas[maxElementIndex] = 0;
        bigContours.push_back( contours[maxElementIndex] );
        drawContours( im_transf, bigContours, (int)i, Scalar(200), 2, LINE_8, hierarchy, 0 );
    }


    std::vector <Point> centers;
    // Getting center points
    for ( int i = 0; i < bigCountourCount; i++ )
    {
        Rect bound = boundingRect( bigContours[i] );
        Point c;
        c.x = (bound.br().x + bound.tl().x) / 2;
        c.y = (bound.br().y + bound.tl().y) / 2;
        centers.push_back(c);
    }

    // Unify centers
    Point g_center;
    for ( int i = 0; i < bigCountourCount; i++ )
    {
        g_center.x += centers[i].x;
        g_center.y += centers[i].y;
    }

    g_center.x = g_center.x / bigCountourCount;
    g_center.y = g_center.y / bigCountourCount;

    circle(im_transf, g_center, 4, Scalar(0,200,0), 4);
    imshow( "Contours", im_transf );
    return g_center;
}

MainWindow::~MainWindow()
{
}

