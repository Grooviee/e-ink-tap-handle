use <libs/threads.scad>
width = 40;
handle_height=60;
mm_per_inch = 25.4;
$fn=100;

    difference() {
        rotate([0,90,180]) {
            union(){ 
                import("./TTGO_T5_Enclosure_-_Box.stl");

                translate([33.5,-(width/2),0]){
                    cube([2.5, width, 7.5]); //Petal base
                    hull() {
                        translate([2.5,0,0]) cube([1, width, 10]); //Petal base
                        translate([40,width/2,0]) cylinder(r = 8, h = 14); 
                        translate([48,width/2-6,0]) cube([1, 12, 14]); 
                    }
                }
            }
        }
        translate([-7.25,-0.5,-85]){
            english_thread(diameter=3/8, threads_per_inch=16,length=20/mm_per_inch, internal=true);
        }
    }
