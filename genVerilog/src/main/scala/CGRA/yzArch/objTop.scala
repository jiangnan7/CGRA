package CGRA.yzArch
import CGRA.ADL.ADL.{EleTrace, ModuleTrace}
import CGRA.ARCH.moduleIns.CGRAParam.PEParam
import CGRA.ARCH.moduleIns.PE
import CGRA.IR.Info2Xml.dumpIR
import CGRA.element.ALU.OPC._
import CGRA.module.ModuleInfo
import CGRA.module.TopModule.topGen
import CGRA.parameter.EleType._
import Replace.ReplaceModule
object objTop  {
  class ArcTop(col: Int, opList: List[Int], inList: List[Tuple3[Int, Int, Int]], outList: List[Tuple2[Int, Int]], matinxIn: List[List[Tuple2[Int, Int]]])
    extends ModuleTrace("yzArch") {
    this.typeStr = "Top"
    this.deviceStr = "Top"
    this.width = 32

    this.inPorts = (0 until inList.size).map("in" + _.toString).toList

    this.outPorts = (0 until outList.size).map("out" + _.toString).toList
    
    for (i <- 0 until opList.size) {
//      val name = if (opList(i) == 0) "PEADD" else if (opList(i) == 1) "PEMUL" else "PEADDMUL"
//      val peOp = if (opList(i) == 0) List(ADD) else if (opList(i) == 1) List(MUL) else List(ADD, MUL)
      val name = opList(i) match {
        case 0 => "PEADD"
        case 1 => "PEMUL"
        case 2 => "PESUB"
        case 3 => "PEADDMUL"
        case 4 => "PEADDSUB"
        case 5 => "PEMULSUB"
        case _ => "PEADDMULSUB"
      }
      val peOp = opList(i) match {
        case 0 => List(ADD)
        case 1 => List(MUL)
        case 2 => List(SUB)
        case 3 => List(ADD, MUL)
        case 4 => List(ADD, SUB)
        case 5 => List(MUL, SUB)
        case _ => List(ADD, MUL ,SUB)
      }
      for (j <- 0 until col) {
        val peParam = PEParam(peOp, width, name)
        addModule(PE(peParam, "PE" + i.toString + "_" + j.toString))
      }
    }


    for (i <- 0 until opList.size) {
      for (j <- 0 to 1) {
        val moduleT = new ModuleTrace("Matrix" + i.toString + "_" + j.toString, "Matrix", "Matrix" + i.toString
          , width, (0 until matinxIn(i).size).map { i => "in" + i.toString }.toList, (0 until col).map { i => "out" + i.toString }.toList)
        println("modulename is " + "Matrix" + i.toString + "_" + j.toString)
        addModule(moduleT)
      }
    }


    for (i <- 0 until opList.size) {
      for (j <- 0 until col) {
        for (k <- 0 to 1) {
          if (inList.contains(i, j, k)) {
            val nuxIns = new EleTrace("Mux" + i.toString + "_" + j.toString + "_" + k.toString, TYPE_Multiplexer.id, List("in0", "in1"), List("out0"), List(2, width))
            addEle(nuxIns)
            val inNum = inList.indexOf((i, j, k))
            addConnect(("this", "in" + inNum.toString), ("Mux" + i.toString + "_" + j.toString + "_" + k.toString, "in0"))
            addConnect(("Matrix" + i.toString + "_" + k.toString, "out" + j.toString), ("Mux" + i.toString + "_" + j.toString + "_" + k.toString, "in1"))
            addConnect(("Mux" + i.toString + "_" + j.toString + "_" + k.toString, "out0"), ("PE" + i.toString + "_" + j.toString, "in" + k.toString))
          } else {
            addConnect(("Matrix" + i.toString + "_" + k.toString, "out" + j.toString), ("PE" + i.toString + "_" + j.toString, "in" + k.toString))
          }
        }
      }
    }

    for (p <- 0 until matinxIn.size) {
      for (q <- 0 until matinxIn(p).size) {
        addConnect(("PE" + matinxIn(p)(q)._1.toString + "_" + matinxIn(p)(q)._2.toString, "out0"), ("Matrix" + p.toString + "_" + 0.toString, "in" + q.toString))
        addConnect(("PE" + matinxIn(p)(q)._1.toString + "_" + matinxIn(p)(q)._2.toString, "out0"), ("Matrix" + p.toString + "_" + 1.toString, "in" + q.toString))
      }
    }


    for (i <- 0 until outList.size) {
      addConnect(("PE" + outList(i)._1.toString + "_" + outList(i)._2.toString, "out0"), ("this", "out" + i.toString))
    }


  }


  def TopYZGen(
                col: Int, 
                opList: List[Int],
                inList: List[Tuple3[Int, Int, Int]],
                outList: List[Tuple2[Int, Int]],
                matinxIn: List[List[Tuple2[Int, Int]]],
                matrixInner : List[scala.collection.mutable.Map[List[Int] , List[List[Int]]]],
                muxNum : Int 
              ):ModuleInfo  ={
    var moduleInit = new ArcTop(col,opList,inList,outList,matinxIn).getModuleInfo()
    for ( i <- matrixInner.indices){
//      val matrix = moduleInit.getModuleList()(i)
      val matrixInfo = new  connectinfo("Matrix" ,"Matrix" + i.toString,
        (0 until matinxIn(i).size).map { i => "in" + i.toString }.toList, (0 until col).map { i => "out" + i.toString}.toList,
        matrixInner(i),moduleInit.getWidth())
      val matrixNew = matrixInfo.product_mulin(muxNum,0)
      moduleInit = ReplaceModule(moduleInit , "Matrix" + i.toString,matrixNew)
    }
    moduleInit
  }

}


object yazhouGen extends App {
  val arcNew = objTop.TopYZGen(
    4,
    List(6, 6, 6, 6),
    List(
      (0, 0, 0),
      (0, 0, 1),
      (0, 1, 0),
      (0, 1, 1),
      (0, 2, 0),
      (0, 2, 1),
      (0, 3, 0),
      (0, 3, 1),
      (1, 0, 0),
      (1, 0, 1),
      (1, 1, 0),
      (1, 1, 1),
      (1, 2, 0),
      (1, 2, 1),
      (1, 3, 0),
      (1, 3, 1),
      (2, 0, 0),
      (2, 0, 1),
      (2, 1, 0),
      (2, 1, 1),
      (2, 2, 0),
      (2, 2, 1),
      (2, 3, 0),
      (2, 3, 1),
      (3, 0, 0),
      (3, 0, 1),
      (3, 1, 0),
      (3, 1, 1),
      (3, 2, 0),
      (3, 2, 1),
      (3, 3, 0),
      (3, 3, 1)
    ),
    List(
      (0, 0),
      (0, 1),
      (0, 2),
      (0, 3),
      (1, 0),
      (1, 1),
      (1, 2),
      (1, 3),
      (2, 0),
      (2, 1),
      (2, 2),
      (2, 3),
      (3, 0),
      (3, 1),
      (3, 2),
      (3, 3)
    ),
    List(
      List(
        (0, 1),
        (0, 3),
        (1, 0),
        (3, 0),
        (0, 2),
        (0, 0),
        (1, 1),
        (3, 1),
        (3, 3),
        (2, 3),
        (3, 2),
        (1, 3),
        (2, 2),
        (2, 1),
        (1, 2)
      ),
      List(
        (1, 1),
        (1, 3),
        (2, 0),
        (0, 0),
        (1, 0),
        (1, 2),
        (0, 1),
        (2, 1),
        (3, 3),
        (3, 1),
        (3, 2),
        (2, 2),
        (0, 2),
        (2, 3),
        (0, 3)
      ),
      List(
        (2, 1),
        (2, 3),
        (3, 0),
        (1, 0),
        (2, 2),
        (3, 1),
        (3, 2),
        (2, 0),
        (1, 1),
        (3, 3),
        (1, 2),
        (1, 3)
      ),
      List(
        (3, 1),
        (3, 3),
        (0, 0),
        (2, 0),
        (3, 0),
        (3, 2),
        (0, 1),
        (2, 1),
        (2, 3),
        (0, 2),
        (2, 2),
        (0, 3)
      )
    ),

    List(
      scala.collection.mutable.Map(
        List(8, 0) -> List(List(8, 0, 1), List(8, 1, 1), List(8, 2, 1), List(8, 3, 1), List(8, 4, 1)),
        List(8, 1) -> List(List(8, 4, 1), List(8, 5, 1), List(8, 6, 1), List(8, 7, 1), List(8, 8, 1), List(8, 9, 1), List(8, 10, 1), List(8, 11, 2), List(8, 12, 1), List(8, 13, 1)),
        List(8, 2) -> List(List(8, 1, 2), List(8, 0, 1), List(8, 14, 1), List(8, 10, 2), List(8, 12, 2), List(8, 11, 1), List(8, 8, 1), List(8, 7, 1)),
        List(8, 3) -> List(List(8, 5, 1), List(8, 4, 1), List(8, 11, 1), List(8, 8, 1), List(8, 9, 1))
      ),
      scala.collection.mutable.Map(
        List(8, 0) -> List(List(8, 0, 1), List(8, 1, 1), List(8, 2, 1), List(8, 3, 1)),
        List(8, 1) -> List(List(8, 4, 1), List(8, 5, 1), List(8, 6, 1), List(8, 7, 1), List(8, 8, 1), List(8, 1, 1), List(8, 9, 2), List(8, 10, 1), List(8, 11, 1)),
        List(8, 2) -> List(List(8, 0, 1), List(8, 1, 1), List(8, 12, 1), List(8, 11, 1)),
        List(8, 3) -> List(List(8, 4, 1), List(8, 5, 1), List(8, 13, 1), List(8, 14, 1), List(8, 9, 1), List(8, 10, 2), List(8, 8, 2))
      ),
      scala.collection.mutable.Map(
        List(8, 0) -> List(List(8, 0, 2), List(8, 1, 1), List(8, 2, 1), List(8, 3, 1), List(8, 4, 1), List(8, 5, 2), List(8, 6, 2)),
        List(8, 1) -> List(List(8, 7, 1), List(8, 4, 1), List(8, 8, 1), List(8, 5, 1), List(8, 9, 1), List(8, 1, 1)),
        List(8, 2) -> List(List(8, 0, 1), List(8, 1, 2), List(8, 10, 1), List(8, 6, 2), List(8, 5, 1), List(8, 9, 1)),
        List(8, 3) -> List(List(8, 7, 1), List(8, 4, 1), List(8, 9, 1), List(8, 11, 1))
      ),
      scala.collection.mutable.Map(
        List(8, 0) -> List(List(8, 0, 1), List(8, 1, 1), List(8, 2, 1), List(8, 3, 1)),
        List(8, 1) -> List(List(8, 4, 1), List(8, 5, 1), List(8, 6, 1), List(8, 7, 1), List(8, 8, 1), List(8, 1, 1)),
        List(8, 2) -> List(List(8, 0, 1), List(8, 1, 1), List(8, 9, 1), List(8, 10, 1)),
        List(8, 3) -> List(List(8, 5, 1), List(8, 4, 1), List(8, 11, 1), List(8, 8, 1))
      )
    ),

    2
  )
  dumpIR(arcNew, "arcNew.xml","arcNew")
//    //  val pe = new PE(PEParam(List(OPC.ADD) , 32,"PETest"),"PE")
//
//    val arcYZ = new ArcTop(1 , List(0), List((0,0,0)) , List((0,0)),List(
//      List((0,0))
//    ))
//  infooutput("arcNew.scala" , arcNew)
    chisel3.Driver.execute(args,() => topGen(arcNew, "Test.txt") )
//  //  infooutput("arcYZ.scala", arcYZ.getModuleInfo())
//    dumpIR(arcYZ.getModuleInfo(),"arcYZ.xml","arcYZ")
//  //  chisel3.Driver.execute(args,() => topGen(PE(PEParam(List(OPC.ADD) , 32,"PETest"),"PE").getModuleInfo(), "PETest.txt") )//生成verilog
}
