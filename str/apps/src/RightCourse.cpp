/**
 * @file RightCourse.cpp
 * @brief RightCourseクラスの関数を定義<br>
 * @author Futa HIRAKOBA
 */
#include "RightCourse.h"
#include "BasicWalker.h"
#include "Distinguisher.h"
#include "Lifter.h"
#include "LineTracerWalker.h"

/**
 *Rコースの走行範囲の切り替えを行う
 */
void RightCourse::run(std::int16_t brightness, std::int16_t black, std::int16_t white,
                      std::int16_t gray)
{
  LineTracerWalker lineTracer;
  runNormalCourse(brightness, black, white, gray);
  //controller.tslpTsk(400);
  solveBlockPuzzle(brightness);
  runParking(brightness, lineTracer, black, white);
}

void RightCourse::solveBlockPuzzle(std::int16_t brightness)
{
  BlockSolver blockSolver{ controller, walker, initialPositionCode, brightness };
  blockSolver.run();
}

void RightCourse::runParking(std::int16_t brightness, LineTracerWalker lineTracer,
                             std::int16_t black, std::int16_t white)
{
  Parking parking{ controller };
  parking.runParpendicular(brightness, lineTracer, black, white);
}

void RightCourse::moveBlockAreaTo8(std::int16_t target_brightness)
{
  Distinguisher d{ controller };
  MotorAngle motor_angle;
  bool isAlreadyChangedGear = false;

  lineTracer.speedControl.setPid(5.0, 1.0, 0.1, 90.0);
  lineTracer.turnControl.setPid(2.0, 1.0, 0.14, target_brightness);
  controller.tslpTsk(500);

  walker.reset();
  while(1) {
    Color result = d.getColor();
    auto luminance = controller.getBrightness();
    lineTracer.runLine(walker.get_count_L(), walker.get_count_R(), luminance);
    walker.run(lineTracer.getForward(), lineTracer.getTurn());
    controller.printDisplay(4, "Brightness: %d, Target: %d", luminance, result);
    if(result == Color::RED) break;
    if(!isAlreadyChangedGear
       && motor_angle.absoluteAngleMean(walker.get_count_L(), walker.get_count_R()) > 1750) {
      lineTracer.speedControl.setPid(5.0, 1.0, 0.1, 25.0);
      lineTracer.turnControl.setPid(1.0, 1.0, 0.14, target_brightness);
      controller.speakerPlayTone(controller.noteFs6, 100);
      isAlreadyChangedGear = true;
    }
    controller.tslpTsk(4);
  }
}

void RightCourse::runPuzzle(std::int16_t target_brightness)
{
  Controller controller;
  Distinguisher d{ controller };
  LineTracerWalker lineTracerWalker;

  lineTracerWalker.speedControl.setPid(1.0, 0.8, 0.8, 60.0);
  lineTracerWalker.turnControl.setPid(1.0, 1.7, 0.6, target_brightness);
  walker.reset();
  Color result = d.getColor();
  controller.printDisplay(6, "in the loop, Color: %d", static_cast<int>(result));
  controller.speakerPlayTone(controller.noteFs4, 100);
  while(1) {
    Color result = d.getColor();
    auto luminance = controller.getBrightness();
    lineTracerWalker.runLine(walker.get_count_L(), walker.get_count_R(), luminance);
    walker.run(lineTracerWalker.getForward(), lineTracerWalker.getTurn());
    controller.printDisplay(4, "Brightness: %d, Target: %d", luminance, result);
    controller.printDisplay(6, "out the loop, Color: %d", static_cast<int>(result));
  }
  controller.tslpTsk(4);
}

void RightCourse::throughArea()
{
  //ブロックエリアを通り過ぎる関数
  BasicWalker basic{ controller };
  Controller controller;
  controller.speakerPlayTone(controller.noteFs4, 100);
  basic.reset();
  basic.setPidWithoutTarget(5.0, 1.0, 0.1);
  basic.goStraight(15, 140);
}

void RightCourse::runNormalCourse(std::int16_t brightness, std::int16_t black, std::int16_t white,
                                  std::int16_t gray)
{
  RightNormalCourse normalCourse;
  bool isNormalCourse;

  /*灰色を検知用*/
  /*
  int8_t counter = 0;
  std::int16_t target_brightness_gray = (white + gray) / 2;
  */

  // NormalCourseを抜けるまでループする
  while(1) {
    auto luminance = controller.getBrightness();

    /*灰色を検知用*/
    /*
    auto distance_total_r = (walker.get_count_L() + walker.get_count_R()) / 2;
    */
    controller.printDisplay(4, "Brightness: %d, Target: %d", luminance, brightness);
    if(normalCourse.statusCheck(walker.get_count_L(), walker.get_count_R()))
      controller.speakerPlayTone(controller.noteFs6, 100);
    isNormalCourse = normalCourse.runNormalCourse(brightness, black, white, gray);
    normalCourse.lineTracerWalker.runLine(walker.get_count_L(), walker.get_count_R(), luminance);

    normalCourse.runOrStop(walker);
    if(!isNormalCourse) {
      walker.run(0, 0);
      break;
    }
    if(controller.buttonIsPressedBack()) {
      walker.run(0, 0);
      break;
    }
    /*灰色を検知したら止まる*/
    /*
    const std::std::int16_t AFTER_GOAL_CURVE_R = 11900;
      if(distance_total_r > AFTER_GOAL_CURVE_R && target_brightness_gray + 20 > luminance &&
    luminance > target_brightness_gray - 3){ if(counter > 10){ controller.printDisplay(4, "Find Gray
    Line¥n Brightness: %d, Target: %d", luminance, brightness);
          controller.speakerPlayTone(controller.noteFs6, 100);
        }
        controller.speakerPlayTone(controller.noteFs6, 20);
        counter++;
      }else{
        counter = 0;
      }
    */
    controller.tslpTsk(4);  // 4msec周期起動
  }
}
