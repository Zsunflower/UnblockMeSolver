//
// Created by CUONG on 12/3/2018.
//

#include "Preprocessing.h"
#include "cocos2d.h"

using namespace cv;

Rect extract_board(Mat &image)
{
    Mat edges;
    Canny(image, edges, 100, 200);

#ifdef DEBUG
    imwrite("edges_image.jpg", edges);
#endif // DEBUG

    vector<vector<Point> > contours;
    vector<Vec4i> hierarchy;
    findContours(edges, contours, hierarchy, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
    int max_area = -1;
    Rect table_rect;
    for(auto &c : contours)
    {
        Rect r = boundingRect(c);
        if(r.width * r.height > max_area)
        {
            max_area = r.width * r.height;
            table_rect = r;
        }
    }

    return table_rect;
}


void extract_blocks(Mat &board, vector<Rect> &_blocks)
{
    Mat edges;
    Canny(board, edges, 100, 240);
    vector<vector<Point> > contours;
    vector<Vec4i> hierarchy;

    findContours(edges, contours, hierarchy, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);

#ifdef DEBUG
    Mat	blocks;
    cvtColor(board, blocks, COLOR_BGR2GRAY);
    cvtColor(blocks, blocks, COLOR_GRAY2BGR);
#endif // DEBUG

    for(auto &c : contours)
    {
        Rect rect = boundingRect(c);
        if(rect.width > 100 && rect.height > 100 && rect.width < 0.75 * board.cols)
            _blocks.push_back(rect);
    }
#ifdef DEBUG
    for (auto r : _blocks)
        rectangle(blocks, r, Scalar(255, 0, 255, 255), 2);

    imwrite("edges_board.jpg", edges);
    imwrite("blocks.jpg", blocks);
#endif // DEBUG
}

void estimate_block_position(
        vector<Rect> &blocks, vector<vector<Point>> &block_position, vector<int> &_red_block_indx,
        Point exit_pos)
{
    if(blocks.empty())
        return;
    int element_size;
    int x, y, xd, yd;
    int c = 0;
    for(auto &block : blocks)
    {
        element_size = min(block.width, block.height);
        x = (block.x) / element_size;
        y = (block.y) / element_size;
        xd = x + block.width / element_size;
        yd = y + block.height / element_size;
        if(y == exit_pos.y && yd == exit_pos.y + 1)
            _red_block_indx.push_back(c);

#ifdef DEBUG
        cout << x << "," << y << "," << xd << "," << yd << " -> ";
        cout << (block.width > block.height ? block.width / block.height : block.height / block.width) << endl;
#endif // DEBUG

        vector<Point> block_pos;
        for(int i = x; i < xd; ++i)
        {
            for(int j = y; j < yd; ++j)
            {
                block_pos.push_back(Point(i, j));
            }
        }
        if(block_pos.size() > 1 && block_pos.size() < 4)
        {
            block_position.push_back(block_pos);
            ++c;
        }
    }
}

void parseScreenShot(const char *path, vector<vector<Point>> &block_pos)
{
    Mat mat = imread(path);
    Rect table_rect = extract_board(mat);
    table_rect.x += 2;
    table_rect.y += 2;
    table_rect.width -= 2;
    table_rect.height -= 4;
#ifdef DEBUG
    Mat log_mat = mat.clone();
    cout << table_rect.x << ", " << table_rect.y << ", " << table_rect.width << ", " << table_rect.height << "\n";
    rectangle(log_mat, table_rect, (255, 0, 255, 255), 1);
    imwrite("board.jpg", log_mat);
#endif // DEBUG

    Mat board = mat(table_rect);
    vector<Rect> blocks;
    extract_blocks(board, blocks);

    vector<int> red_block_ind;
    estimate_block_position(blocks, block_pos, red_block_ind);

#ifdef DEBUG
    LOGD << "------------------------ Blocks:-------------------\n";
    for (auto &block : block_pos)
    {
        for (auto &p : block)
            cout << "(" << p.x << "," << p.y << "), ";
        cout << "\n";
    }
#endif // DEBUG

    int red_index = -1, max_x = -1;
    for(auto i : red_block_ind)
    {
        if(block_pos[i].front().x > max_x)
        {
            red_index = i;
            max_x = block_pos[i].front().x;
        }
    }
#ifdef DEBUG
    cout << "-------------- Red block index : ----------------" << red_index << "\n";
    for (auto p : block_pos[red_index])
        cout << "(" << p.x << ", " << p.y << ")";
    cout << endl;
#endif // DEBUG

    swap(block_pos[red_index], block_pos.back());

#ifdef DEBUG
    cout << "--------------- Blocks: -----------------\n";
    for (auto &block : block_pos)
    {
        for (auto &p : block)
            cout << "(" << p.x << "," << p.y << "), ";
        cout << "\n";
    }
#endif // DEBUG
}

inline float sqr(float x)
{
    return x * x;
}

inline float distance(Point f, Point s)
{
    return sqrt(sqr(f.x - s.x) + sqr(f.y - s.y));
}

void search_for_block(Mat &mat, Mat &templ, vector<Rect> &blocks)
{
    int brow, bcol, result_cols, result_rows;
    int i;
    brow = templ.rows;
    bcol = templ.cols;
    result_cols = mat.cols - templ.cols + 1;
    result_rows = mat.rows - templ.rows + 1;
    Mat res_mat, nonZeros;
    if (result_cols < 50 || result_rows < 50)
        return;
    res_mat.create(result_rows, result_cols, CV_32FC1);
    matchTemplate(mat, templ, res_mat, TM_CCOEFF_NORMED);

    threshold(res_mat, res_mat, 0.95, 1.0, THRESH_BINARY);
    res_mat.convertTo(nonZeros, CV_8UC1);
    Mat match_pos;
    findNonZero(nonZeros, match_pos);
    for(i = 0; i < match_pos.total(); ++i)
    {
        blocks.push_back(Rect(match_pos.at<Point>(i).x, match_pos.at<Point>(i).y, bcol, brow));
    }
}

void search_for_red_block(Mat &mat, Mat &templ, Rect &red_block)
{
    int brow, bcol, result_cols, result_rows;
    double minVal, maxVal;
    Point minLoc, maxLoc;

    brow = templ.rows;
    bcol = templ.cols;
    result_cols = mat.cols - templ.cols + 1;
    result_rows = mat.rows - templ.rows + 1;
    Mat res_mat;
    if (result_cols < 50 || result_rows < 50)
        return;
    res_mat.create(result_rows, result_cols, CV_32FC1);
    matchTemplate(mat, templ, res_mat, TM_CCOEFF_NORMED);
    minMaxLoc(res_mat, &minVal, &maxVal, &minLoc, &maxLoc, Mat());

    red_block.x = maxLoc.x;
    red_block.y = maxLoc.y;
    red_block.width = bcol;
    red_block.height = brow;
}


void filter_block(vector<Rect> &all_blocks, vector<Rect> &unq_blocks)
{
    bool dup;
    for(auto &r : all_blocks)
    {
        dup = false;
        Point ct;
        ct.x = r.x + r.width / 2;
        ct.y = r.y + r.height / 2;
        for(auto &ur : unq_blocks)
        {
            Point uct;
            uct.x = ur.x + ur.width / 2;
            uct.y = ur.y + ur.height / 2;
            if(distance(ct, uct) < 20)
            {
                dup = true;
                break;
            }
        }
        if(!dup)
            unq_blocks.push_back(r);
    }
}

void cal_block_position(vector<Rect> &blocks, vector<vector<Point>> &block_position)
{
    if(blocks.size() < 2)
        return;
    int element_size;
    int x, y, xd, yd;
    for(auto &block : blocks)
    {
        element_size = min(block.width, block.height);
        x = (block.x) / element_size;
        y = (block.y) / element_size;
        xd = x + block.width / element_size;
        yd = y + block.height / element_size;

#ifdef DEBUG
        cout << x << "," << y << "," << xd << "," << yd << " -> ";
        cout << (block.width > block.height ? block.width / block.height : block.height / block.width) << endl;
#endif // DEBUG

        vector<Point> block_pos;
        for(int i = x; i < xd; ++i)
        {
            for(int j = y; j < yd; ++j)
            {
                block_pos.push_back(Point(i, j));
            }
        }
        if(block_pos.size() > 1 && block_pos.size() < 4)
            block_position.push_back(block_pos);
    }
}

void processScreenShot(const char *path, vector<vector<Point>> &block_position)
{
    vector<Rect> blocks, unq_blocks;
    Rect red_block;

    Mat mat = imread(path);
    Mat templ1 = imread(cache_dir + "2blockh.png");
    Mat templ2 = imread(cache_dir + "2blockv.png");
    Mat templ3 = imread(cache_dir + "3blockh.png");
    Mat templ4 = imread(cache_dir + "3blockv.png");
    Mat red = imread(cache_dir + "red.png");
    if(mat.data == nullptr || templ1.data == nullptr || templ2.data == nullptr ||
       templ3.data == nullptr || templ4.data == nullptr || red.data == nullptr)
    {
        cocos2d::log("UBMSolver [=] ============= Read images failed: %s ============= [=]", cache_dir.c_str());
        return;
    }
    resize(mat, mat, Size(), 0.5, 0.5);
    resize(templ1, templ1, Size(), 0.5, 0.5);
    resize(templ2, templ2, Size(), 0.5, 0.5);
    resize(templ3, templ3, Size(), 0.5, 0.5);
    resize(templ4, templ4, Size(), 0.5, 0.5);
    resize(red, red, Size(), 0.5, 0.5);
    Rect table_rect = extract_board(mat);

    mat = mat(table_rect);
    Mat mat_gray, templ1_gray, templ2_gray, templ3_gray, templ4_gray, red_gray;
    cvtColor(mat, mat_gray, COLOR_BGRA2GRAY);
    cvtColor(templ1, templ1_gray, COLOR_BGRA2GRAY);
    cvtColor(templ2, templ2_gray, COLOR_BGRA2GRAY);
    cvtColor(templ3, templ3_gray, COLOR_BGRA2GRAY);
    cvtColor(templ4, templ4_gray, COLOR_BGRA2GRAY);
    cvtColor(red, red_gray, COLOR_BGRA2GRAY);

    search_for_block(mat_gray, templ1_gray, blocks);
    search_for_block(mat_gray, templ2_gray, blocks);
    search_for_block(mat_gray, templ3_gray, blocks);
    search_for_block(mat_gray, templ4_gray, blocks);

    search_for_red_block(mat_gray, red_gray, red_block);

    cocos2d::log("Search found: %d", blocks.size());
    filter_block(blocks, unq_blocks);
    cocos2d::log("After filtered: %d", unq_blocks.size());

#ifdef DEBUG
    for (auto &r : unq_blocks)
        rectangle(mat, r, Scalar(0, 0, 255, 255), 3);
    rectangle(mat, red_block, Scalar(0, 255, 0, 255), 3);
    imwrite("blocks.png", mat);
#endif

    unq_blocks.push_back(red_block);
    cal_block_position(unq_blocks, block_position);

    for(auto block : block_position)
    {
        cocos2d::log("-------------------------------------");
        for(auto c : block)
        {
            cocos2d::log("(%d , %d)", c.x, c.y);
        }
        cocos2d::log("-------------------------------------");
    }
}