/*
 * DSL test file for the AML analysis feature.
 */
DefinitionBlock ("", "DSDT", 2, "TEST00", "881D    ", 0x00000002)
{
  Name (FFOO, One)

  /*
   * Method \M001
   *
   * Test method containing two similar Switch/Case patterns within the same
   * scope. Both patterns use identical Switch() and Case() statements and
   * differ only in the final Return() statement.
   *
   * This method is used to verify that the fingerprint matching algorithm
   * correctly performs backtracking when multiple candidate matches exist.
   */
  Method (M001, 1, Serialized)
  {
    Switch (Arg0)
    {
      Case (Zero)
      {
        Return (Zero)
      }

      Case (One)
      {
        Return (Zero)
      }
    }

    Switch (Arg0)
    {
      Case (Zero)
      {
        Return (Zero)
      }

      Case (One)
      {
        Return (FFOO) /* different */
      }
    }

    Return (Zero)
  }

  Scope (_SB)
  {
    /*
     * Method \M002
     *
     * Test method containing nested Switch/Case patterns with identical Case()
     * values at different nesting levels.
     *
     * This method is used to verify the behavior of the '..' and '...'
     * wildcard operators. The '..' operator should only match direct child
     * Case() blocks of a Switch(), while the '...' operator should match
     * Case() blocks at any nesting level within the Switch() scope.
     */
    Method (^M002, 2, Serialized)
    {
      Switch (Arg0)
      {
        Case (Zero)
        {
          Switch (Arg1)
          {
            Case (Zero)
            {
              Return (Zero)
            }

            Case (One)
            {
              Return (FFOO)
            }
          }
        }

        Case (One)
        {
          Switch (Arg1)
          {
            Case (Zero)
            {
              Return (Zero)
            }

            Case (One)
            {
              Return (FFOO)
            }
          }
        }
      }

      Return (Zero)
    }

    /*
     * Method \_SB.M003
     *
     * Test method located within the \_SB scope.
     *
     * This method is used to verify correct namespace handling and scope
     * resolution during AML analysis.
     */
    Method (M003, 0, Serialized)
    {
      Return (Zero)
    }

    Scope (\)
    {
      /*
       * Method \M004
       *
       * Test method containing a single If statement.
       *
       * The iasl disassembler emits comparison expressions inside If statements
       * with an additional pair of parentheses. This method is used to verify
       * that redundant parentheses are removed correctly during analysis.
       */
      Method (M004, 1, Serialized)
      {
        If (Arg0 == 0xFF)
        {
          Return (0xFF)
        }

        Return (Zero)
      }
    }
  }
}
