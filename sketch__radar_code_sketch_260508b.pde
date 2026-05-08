import processing.serial.*;

Serial myPort;
int[] distances = {40, 40, 40, 40}; // مصفوفة لتخزين المسافات للجهات الأربع
float angle = 0; // زاوية دوران مؤشر الرادار

void setup() {
  size(800, 800); // حجم نافذة الرادار على الشاشة
  smooth();
  
  // تتبع الأردوينو عبر منفذ الـ USB
  // تأكدي من كتابة منفذ الـ COM الفعلي الخاص بجهازكِ هنا (مثال: COM4)
  String portName = "COM5"; 
  myPort = new Serial(this, portName, 9600);
  myPort.bufferUntil('\n'); // القراءة عند نهاية السطر
}

void draw() {
  // استخدام خلفية سوداء مع تأثير شفافية بسيط (Fade) لإعطاء توهج ذيل الرادار كالمحترفين
  fill(0, 0, 0, 15); 
  noStroke();
  rect(0, 0, width, height);
  
  translate(width/2, height/2); // نقل نقطة الصفر إلى منتصف الشاشة
  
  // 1. رسم شبكة الرادار الدائرية الخضراء
  noFill();
  strokeWeight(2);
  stroke(0, 150, 0, 120); // لون أخضر داكن شفاف
  for (int r = 100; r <= 350; r += 100) {
    ellipse(0, 0, r*2, r*2);
    
    // كتابة المسافات التوضيحية على الدوائر (10 متر، 20 متر، 30 متر)
    fill(0, 150, 0, 150);
    textSize(12);
    text(r/10 + "0m", r - 15, 15);
    noFill();
  }
  
  // رسم خطوط المحاور الأربعة
  stroke(0, 100, 0, 80);
  line(-380, 0, 380, 0);
  line(0, -380, 0, 380);
  
  // كتابة أسماء الاتجاهات بالإنجليزية الفخمة (لتفادي مشكلة المربعات العربية)
  fill(0, 255, 0);
  textSize(16);
  textAlign(CENTER, CENTER);
  text("NORTH", 0, -370);
  text("EAST", 340, 20);
  text("SOUTH", 0, 370);
  text("WEST", -340, 20);
  
  textSize(18);
  text("SMART HOME RADAR SYSTEM", 0, -20);
  
  // 2. رسم الأهداف المكتشفة باللون الأحمر التفاعلي المتوهج
  strokeWeight(12);
  
  // الشمال (North)
  if (distances[0] < 40) {
    float y = map(distances[0], 0, 40, 0, -350);
    stroke(255, 0, 0, 200); // نقطة حمراء متوهجة
    point(0, y);
  }
  
  // الشرق (East)
  if (distances[1] < 40) {
    float x = map(distances[1], 0, 40, 0, 350);
    stroke(255, 0, 0, 200);
    point(x, 0);
  }
  
  // الجنوب (South)
  if (distances[2] < 40) {
    float y = map(distances[2], 0, 40, 0, 350);
    stroke(255, 0, 0, 200);
    point(0, y);
  }
  
  // الغرب (West)
  if (distances[3] < 40) {
    float x = map(distances[3], 0, 40, 0, -350);
    stroke(255, 0, 0, 200);
    point(x, 0);
  }
  
  // 3. رسم خط مسح الرادار الدوار (Sweeping Line)
  strokeWeight(3);
  stroke(0, 255, 0, 220); // خط أخضر مضيء وقوي
  float xEnd = 350 * cos(angle);
  float yEnd = 350 * sin(angle);
  line(0, 0, xEnd, yEnd);
  
  // تحديث زاوية الدوران ليتحرك الخط باستمرار
  angle += 0.05; 
  if (angle > TWO_PI) {
    angle = 0;
  }
}

// دالة استقبال القراءات القادمة من الأردوينو وتحديث المسافات
void serialEvent(Serial myPort) {
  String inString = myPort.readStringUntil('\n');
  if (inString != null) {
    inString = trim(inString);
    int[] temp = int(split(inString, ','));
    if (temp.length == 4) {
      distances[0] = temp[0]; // الشمال
      distances[1] = temp[1]; // الشرق
      distances[2] = temp[2]; // الجنوب
      distances[3] = temp[3]; // الغرب
    }
  }
}
